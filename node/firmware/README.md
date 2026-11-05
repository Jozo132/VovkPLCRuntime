# Firmware builder

Node.js 20+, `npm install`, and an installed PlatformIO CLI are required. Imports do not start a service or install tools. Builds may cause PlatformIO to download the project-specified platforms, frameworks, libraries, and toolchains. Allow sufficient disk space; each job uses a separate PlatformIO core/cache directory, preventing concurrent jobs from modifying a shared cache. PlatformIO and dependencies must be available to the execution environment. Project configuration is preserved, apart from setting an isolated build directory and explicitly selecting the `buildprog` target. The runtime library is never injected or upgraded by the builder.

**Security:** the local executor is not a sandbox. Although submitted archives are checked for scripts, PlatformIO platforms, downloaded dependencies, compiler options and toolchains can still execute code with the builder's privileges. This policy does not inspect downloaded packages or make untrusted projects safe. Hosted services must use a suitably isolated worker/container/VM with resource, network, filesystem and process controls, and must authenticate and authorize every operation. Use the local executor only for trusted projects. An executor replacement must enforce its own isolation and process-tree cleanup.

## Project policy

Archives must contain basic source and configuration files only: C/C++ and Arduino sources and headers, assembly and linker files, CSV, Markdown, plain text, INI, JSON, YAML, XML and properties files. Common extensionless documentation and Git ignore/attributes files are accepted. Unsupported extensions, binary or shebang content, Git and PlatformIO caches, executable scripts, `CMakeLists.txt`, PlatformIO `extra_scripts`, `extra_configs`, command-generated (`!`) configuration, and script hooks in library or package manifests are rejected. The effective PlatformIO configuration is checked again before the build. Validation errors appear in the failed job's status and events. This intentionally excludes ESP-IDF projects requiring project CMake scripts. Custom loader or adapter replacements are responsible for enforcing equivalent policy.

## ZIP to artifacts

From the repository root:

```js
import { createFirmwareBuilder } from './node/firmware-builder.js'

const builder = await createFirmwareBuilder({ directory: './firmware-data' })
try {
    const jobId = await builder.submit({ source: './project.zip', environment: 'esp32dev' })
    for await (const event of builder.events(jobId)) {
        if (event.type === 'output') process.stdout.write(Buffer.from(event.payload.data, 'base64'))
        else console.log(event)
    }
    const status = await builder.status(jobId)
    if (status.state !== 'succeeded') throw new Error(status.error?.message)
    console.log(await builder.result(jobId))
    for (const artifact of await builder.listArtifacts(jobId)) {
        console.log(artifact.id, artifact.filename, artifact.size, artifact.sha256)
        const stream = builder.openArtifact(artifact.id)
        // Consume or pipe this readable stream to your own destination.
        stream.resume()
    }
} finally {
    await builder.close()
}
```

Package subpaths are `vovk-plc-runtime/firmware-builder` and `vovk-plc-runtime/firmware-http`. `source` is a filesystem path or Node readable stream. Submission streams and hashes the ZIP before returning the job UUID; compilation runs asynchronously. A ZIP must contain one `platformio.ini`, unless `projectRoot` selects its relative directory. Supply `environment` when multiple PlatformIO environments exist; an ambiguous job fails with `error.availableEnvironments`. Build errors are returned through status and events, not as submission errors.

`status(id)` returns the persisted state and error or result; `result(id)` returns `{ provenance, manifest }` on success or `null` otherwise. `events(id, { after, signal })` is an async iterable replaying events strictly after the sequence and continuing live until the terminal event. Events have `jobId`, `sequence`, `timestamp`, `type`, and `payload`. Output events contain the exact original bytes in base64 (`data`), the stream `channel`, and a best-effort UTF-8 `text` view. Diagnostic events are best effort; ANSI and carriage returns remain in raw output. State events report elapsed milliseconds and a nullable `estimatedPercentage`, not a fabricated completion percentage. `cancel(id)` returns whether cancellation was accepted; once final publication starts, success wins and cancellation returns false. `listArtifacts(id)`, `artifactInfo(artifactId)` and `openArtifact(artifactId)` expose metadata and readable streams, never workspace paths.

PlatformIO build metadata includes `provenance.memoryUsage.ram` and `provenance.memoryUsage.flash`. Each is either `null` when not reported or `{ usedBytes, totalBytes, percentage }`, using the figures printed by PlatformIO. Flash usage refers to the application limit reported by the build, not necessarily the device's total physical flash capacity. These values also appear in the post-build `metadata` event and the HTTP build status result; other adapters may supply different metadata.

`await builder.checkAdapter()` checks PlatformIO availability and returns `{ available, version, error? }` without starting a build.

Artifacts are copied into an immutable UUID-addressed store and published as a set before success. For supported ESP32 esptool uploads, the version 1 manifest derives chip, flash settings, offsets and image paths from PlatformIO's resolved upload command using an SCons dry run; no device is accessed during this step. Required images from PlatformIO's package directory are included in the artifact set. The manifest is `unavailable` if the command cannot be safely interpreted, an image is missing or outside the build and PlatformIO package directories, or flash ranges are invalid. The successful build artifacts remain available. Custom upload commands, unsupported flags and other upload protocols may not produce a manifest. No OTA suitability is inferred from a firmware filename, so `ota` is currently null. Check the board, chip, flash settings, segments, checksums and manifest status before flashing.

Configuration options: `platformioExecutable`, `executor`, `projectLoader`, `jobStore`, `artifactStore`, `adapters`, and `limits`. Defaults are `uploadBytes: 134217728`, `extractBytes: 536870912`, `files: 10000`, `executionMs: 1800000`, `concurrency: 1`, `queue: 100`, and `retentionMs: 604800000`. Limits are positive integers. Time is milliseconds. Set appropriate filesystem quotas as these limits do not bound log or artifact disk usage. `await builder.cleanup({ olderThanMs })` explicitly removes terminal jobs, their artifacts and caches; active and queued jobs are never removed. Cleanup is not automatic. `await builder.close()` cancels active and queued jobs and closes the store lock.

Only one builder may own a storage directory at a time. On restart, complete queued jobs resume; previously active jobs become `interrupted` and are not rerun. Incomplete uploads become interrupted. A service manager should terminate orphaned process groups or isolated workers after an unexpected host process crash; a restarted builder cannot reliably recover their process identities. Keep the data directory private and durable. No built-in encryption or tenant isolation is provided.

## HTTP and SSE

The optional handler never listens on import. It requires an authorization function and returns 403 when that function rejects an action. This example uses a single shared bearer token; production deployments need TLS, per-user/job authorization, upload quotas and an isolated worker.

```js
import http from 'node:http'
import { timingSafeEqual } from 'node:crypto'
import { createFirmwareBuilder } from 'vovk-plc-runtime/firmware-builder'
import { createFirmwareHttpHandler } from 'vovk-plc-runtime/firmware-http'

const token = process.env.FIRMWARE_API_TOKEN
if (!token) throw new Error('FIRMWARE_API_TOKEN is required')
const builder = await createFirmwareBuilder({ directory: './firmware-data' })
const handler = createFirmwareHttpHandler(builder, {
    authorize: ({ request, action, jobId, artifactId }) => {
        const expected = Buffer.from(`Bearer ${token}`)
        const provided = Buffer.from(request.headers.authorization || '')
        return provided.length === expected.length && timingSafeEqual(provided, expected)
    },
})
const server = http.createServer(handler)
server.listen(8080, '127.0.0.1')
```

`POST /builds?environment=esp32dev` streams a raw `application/zip` body and returns `{ jobId }` with HTTP 202. `GET /builds/:id`, `POST /builds/:id/cancel`, and `GET /builds/:id/artifacts` provide status, cancellation, and artifact metadata with download URLs. `GET /artifacts/:artifactId` streams bytes. `GET /builds/:id/events` returns SSE with event IDs; reconnect using `Last-Event-ID` or `?after=<sequence>`. The authorization hook receives the request, action, job ID, and artifact ID where available; unknown artifact IDs have no job ID. It must enforce ownership, including artifact downloads: UUIDs are not credentials. The caller owns server shutdown and should then call `builder.close()`.

## Extension points

An adapter implements async `prepare(context)`, `build(context, metadata)`, and `collect(context, metadata)`; collection returns `{ outputRoot, files, flash? }`, where each file has an absolute `path` within `outputRoot`, optional `role`, `mediaType`, and a root-relative `source`. An optional `manifest(collected, publishedRecords, provenance)` returns manifest data. Register it with `adapters: { name: adapter }`. The provided PlatformIO adapter also has `check({ executor })` for CLI availability and version. Context includes `root`, `buildDir`, `environment`, `env`, `executor`, `signal`, `timeoutMs`, and async `onOutput(channel, buffer)`.

The local executor's `run({ command, args, cwd, env, signal, timeoutMs, onOutput })` returns `{ code, signal, stdout, truncated }`; implementations must handle cancellation and stream output without unbounded buffering. Remote workers need a shared or explicitly transferred workspace and must implement equivalent path confinement. `projectLoader` has the same signature as exported `extractProject` in `project-loader.js`. The filesystem stores in `stores.js` define the job and artifact store interfaces used by the manager; replacements must preserve serialized, durable event append before notification, ordered replay without gaps, atomic artifact publication, and path confinement. The manager currently expects job-store `jobs`, `dir`, `get`, `create`, `append`, `events`, `remove`, `initialize` and artifact-store `initialize`, `publish`, `list`, `info`, `open`, `remove` methods.

Run `npm run test_firmware` for deterministic tests. To attempt a genuine ESP32 Arduino build, run `FIRMWARE_REAL_BUILD=1 node --test --test-name-pattern='real PlatformIO' node/firmware-builder.test.js`; this may download large toolchains into an isolated temporary cache. For a single local test using an existing cache, add `FIRMWARE_TEST_CORE_DIR="$HOME/.platformio"`. Set `FIRMWARE_TEST_TIMEOUT_MS=7200000` for a two-hour execution limit. This test-only cache override must not be used concurrently with other PlatformIO operations and does not change the builder's per-job caches.