import { spawn } from 'node:child_process'
import { StringDecoder } from 'node:string_decoder'

export class LocalExecutor {
    async run({ command, args = [], cwd, env = {}, signal, timeoutMs = 30 * 60 * 1000, onOutput = async () => {}, captureLimit = 2 * 1024 * 1024 }) {
        signal?.throwIfAborted()
        const child = spawn(command, args, { cwd, env: { ...process.env, ...env }, detached: process.platform !== 'win32', windowsHide: true, stdio: ['ignore', 'pipe', 'pipe'] })
        let timedOut = false
        let killTimer
        const kill = force => {
            try {
                if (process.platform === 'win32') {
                    const killer = spawn('taskkill', ['/pid', String(child.pid), '/t', ...(force ? ['/f'] : [])], { stdio: 'ignore', windowsHide: true })
                    killer.on('error', () => {})
                } else process.kill(-child.pid, force ? 'SIGKILL' : 'SIGTERM')
            } catch (error) { if (error.code !== 'ESRCH') throw error }
        }
        const abort = () => { if (!child.pid) return; kill(false); killTimer = setTimeout(() => kill(true), 2000); killTimer.unref() }
        signal?.addEventListener('abort', abort, { once: true })
        const timer = setTimeout(() => { timedOut = true; abort() }, timeoutMs)
        timer.unref()
        let output = ''
        let captured = 0
        const decoder = new StringDecoder('utf8')
        const read = async (stream, channel) => {
            for await (const chunk of stream) {
                if (channel === 'stdout' && captured < captureLimit) { const part = chunk.subarray(0, captureLimit - captured); output += decoder.write(part); captured += part.length }
                await onOutput(channel, chunk)
            }
        }
        const completion = new Promise((resolve, reject) => { child.once('error', reject); child.once('close', (code, childSignal) => resolve({ code, signal: childSignal })) })
        try {
            const [result] = await Promise.all([completion, read(child.stdout, 'stdout'), read(child.stderr, 'stderr')])
            if (timedOut) throw new Error(`Process timed out after ${timeoutMs} ms`)
            signal?.throwIfAborted()
            return { ...result, stdout: output + decoder.end(), truncated: captured >= captureLimit }
        } catch (error) {
            if (child.pid) kill(true)
            await completion.catch(() => {})
            throw error
        } finally {
            if ((signal?.aborted || timedOut) && child.pid) kill(true)
            clearTimeout(timer); clearTimeout(killTimer); signal?.removeEventListener('abort', abort)
        }
    }
}