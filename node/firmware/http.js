import { once } from 'node:events'
import { pipeline } from 'node:stream/promises'

export function createFirmwareHttpHandler(builder, { authorize } = {}) {
    if (typeof authorize !== 'function') throw new Error('An authorization hook is required')
    return async (request, response) => {
        const url = new URL(request.url, 'http://localhost')
        const parts = url.pathname.split('/').filter(Boolean)
        let action
        let jobId
        let artifactId
        if (request.method === 'POST' && parts.length === 1 && parts[0] === 'builds') action = 'submit'
        else if (parts[0] === 'builds' && parts.length >= 2) {
            jobId = parts[1]
            if (request.method === 'GET' && parts.length === 2) action = 'status'
            if (request.method === 'GET' && parts.length === 3 && parts[2] === 'events') action = 'events'
            if (request.method === 'GET' && parts.length === 3 && parts[2] === 'artifacts') action = 'listArtifacts'
            if (request.method === 'POST' && parts.length === 3 && parts[2] === 'cancel') action = 'cancel'
        } else if (request.method === 'GET' && parts[0] === 'artifacts' && parts.length === 2) { action = 'openArtifact'; artifactId = parts[1] }
        if (!action) { response.writeHead(404).end(); return }
        try {
            if (artifactId) { try { jobId = (await builder.artifactInfo(artifactId)).jobId } catch {} }
            if (!await authorize({ request, action, jobId, artifactId })) { response.writeHead(403).end(); return }
            const json = (status, value) => { response.writeHead(status, { 'content-type': 'application/json; charset=utf-8', 'cache-control': 'no-store' }); response.end(JSON.stringify(value)) }
            if (action === 'submit') {
                const id = await builder.submit({ source: request, adapter: url.searchParams.get('adapter') || undefined, environment: url.searchParams.get('environment') || undefined, projectRoot: url.searchParams.get('projectRoot') || undefined })
                json(202, { jobId: id })
            } else if (action === 'status') json(200, await builder.status(jobId))
            else if (action === 'cancel') json(200, { cancelled: await builder.cancel(jobId) })
            else if (action === 'listArtifacts') json(200, (await builder.listArtifacts(jobId)).map(artifact => ({ ...artifact, downloadUrl: `/artifacts/${artifact.id}` })))
            else if (action === 'openArtifact') {
                const stream = builder.openArtifact(artifactId)
                response.writeHead(200, { 'content-type': 'application/octet-stream', 'content-disposition': `attachment; filename="${artifactId}"`, 'cache-control': 'no-store' })
                await pipeline(stream, response)
            } else {
                const afterValue = request.headers['last-event-id'] || url.searchParams.get('after') || '0'
                if (!/^\d+$/.test(afterValue) || !Number.isSafeInteger(Number(afterValue))) { json(400, { error: 'Invalid event sequence' }); return }
                await builder.status(jobId)
                const controller = new AbortController()
                response.once('close', () => controller.abort())
                response.writeHead(200, { 'content-type': 'text/event-stream; charset=utf-8', 'cache-control': 'no-cache', connection: 'keep-alive' })
                response.flushHeaders()
                for await (const event of builder.events(jobId, { after: Number(afterValue), signal: controller.signal })) {
                    if (!response.write(`id: ${event.sequence}\nevent: ${event.type}\ndata: ${JSON.stringify(event)}\n\n`)) await once(response, 'drain', { signal: controller.signal })
                }
                response.end()
            }
        } catch (error) {
            if (response.headersSent) { response.destroy(); return }
            const status = /not found/i.test(error.message) ? 404 : /limit|queue is full/i.test(error.message) ? 413 : 400
            response.writeHead(status, { 'content-type': 'application/json; charset=utf-8', 'cache-control': 'no-store' }).end(JSON.stringify({ error: error.message }))
        }
    }
}