import { parentPort, workerData } from 'node:worker_threads'
import VirtualPLC from './VirtualPLC.js'
import VirtualPLCServer from './VirtualPLCServer.js'

let instance
let server

try {
    instance = await new VirtualPLC(workerData.directory).initialize(lockToken => parentPort.postMessage({ type: 'lock', lockToken }))
    if (workerData.config) instance.configure(workerData.config)
    server = await new VirtualPLCServer(instance).start()
    parentPort.postMessage({ type: 'ready', uid: instance.state.uid, config: instance.state.config, lockToken: instance.lockToken })
    let queue = Promise.resolve()
    parentPort.on('message', message => {
        queue = queue.then(async () => {
            if (message.type === 'shutdown') {
                await server.close()
                instance.close()
                parentPort.close()
            } else if (message.type === 'configure') {
                try {
                    const previous = { ...instance.state.config }
                    const networkChanged = ['host', 'port', 'discoveryPort'].some(key => Object.hasOwn(message.config, key) && message.config[key] !== previous[key])
                    let config
                    if (networkChanged) {
                        instance.validateConfig({ ...previous, ...message.config })
                        await server.close()
                        try {
                            config = instance.configure(message.config)
                            server = await new VirtualPLCServer(instance).start()
                        } catch (error) {
                            try {
                                instance.configure(previous)
                                server = await new VirtualPLCServer(instance).start()
                            } catch (rollbackError) {
                                parentPort.postMessage({ type: 'response', request: message.request, error: `Rebind failed: ${error.message}; rollback failed: ${rollbackError.message}` })
                                instance.close()
                                parentPort.close()
                                return
                            }
                            throw error
                        }
                    } else config = instance.configure(message.config)
                    parentPort.postMessage({ type: 'response', request: message.request, config })
                } catch (error) {
                    parentPort.postMessage({ type: 'response', request: message.request, error: error.message })
                }
            }
        })
    })
} catch (error) {
    if (server) await server.close()
    instance?.close()
    parentPort.postMessage({ type: 'startup-error', error: error.message })
    parentPort.close()
}