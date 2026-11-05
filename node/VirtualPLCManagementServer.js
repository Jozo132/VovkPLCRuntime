import http from 'node:http'

export default class VirtualPLCManagementServer {
    constructor(manager, port = 19615) {
        if (!Number.isInteger(port) || port < 0 || port > 65535) throw new Error('Invalid management port')
        this.manager = manager
        this.port = port
        this.queue = Promise.resolve()
        this.server = http.createServer((request, response) => this.handle(request, response))
    }

    async start() {
        await new Promise((resolve, reject) => {
            this.server.once('error', reject)
            this.server.listen(this.port, '127.0.0.1', () => { this.server.removeListener('error', reject); resolve() })
        })
        this.port = this.server.address().port
        return this
    }

    async handle(request, response) {
        const reply = (status, data) => {
            response.writeHead(status, { 'Content-Type': 'application/json', 'Cache-Control': 'no-store', 'X-Content-Type-Options': 'nosniff' })
            response.end(JSON.stringify(data))
        }
        if (request.headers.origin || !['127.0.0.1', 'localhost'].includes((request.headers.host || '').split(':')[0])) return reply(403, { ok: false, error: 'Local requests only' })
        try {
            const url = new URL(request.url, 'http://127.0.0.1')
            if (request.method === 'GET' && url.pathname === '/instances') return reply(200, { ok: true, instances: this.manager.list() })
            const match = url.pathname.match(/^\/instances\/([a-f0-9-]{36})(?:\/(start|stop))?$/)
            const hasBody = request.method === 'PATCH' || (request.method === 'POST' && url.pathname === '/instances')
            let body
            if (hasBody) {
                if (request.headers['content-type']?.split(';')[0].trim() !== 'application/json') return reply(415, { ok: false, error: 'JSON required' })
                let data = ''
                for await (const chunk of request) {
                    data += chunk
                    if (data.length > 16384) return reply(413, { ok: false, error: 'Request too large' })
                }
                body = JSON.parse(data)
            }
            const action = async () => {
                if (request.method === 'POST' && url.pathname === '/instances') return this.manager.create(body)
                if (match && !match[2] && request.method === 'PATCH') return this.manager.configure(match[1], body)
                if (match && !match[2] && request.method === 'DELETE') return this.manager.remove(match[1])
                if (match && match[2] === 'start' && request.method === 'POST') return this.manager.start(match[1])
                if (match && match[2] === 'stop' && request.method === 'POST') return this.manager.stop(match[1])
                throw new Error('Unknown operation')
            }
            const result = this.queue.then(action)
            this.queue = result.catch(() => {})
            reply(200, { ok: true, result: await result })
        } catch (error) {
            reply(400, { ok: false, error: error.message })
        }
    }

    async close() {
        await new Promise(resolve => this.server.close(resolve))
        await this.queue
    }
}