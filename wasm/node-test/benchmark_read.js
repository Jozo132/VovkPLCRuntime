import VovkPLC, {VovkPLCWorker} from '../dist/VovkPLC.js'
import path from 'path'
import {fileURLToPath} from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

async function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms))
}

// Add global timeout - Removed, handled per benchmark
// const TIMEOUT_MS = 15000;
// setTimeout(() => { ... });

async function benchmark() {
    console.log('::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::')
    console.log(':: VovkPLC Shared Memory Read Benchmark')
    console.log('::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::')

    const wasmPath = path.resolve(__dirname, '../dist/VovkPLC.wasm')
    const worker = await VovkPLCWorker.create(wasmPath, {silent: false, debug: false})

    // Compile empty program
    await worker.compile('nop')

    // ---------------------------------------------------------
    // 1. Benchmark Standard Async Read (Command Protocol)
    // ---------------------------------------------------------
    console.log('\n[1] Benchmarking Standard Async Read (Command Protocol)...')
    console.log('    (Max 5 seconds)')

    // Warmup
    await worker.readMemoryArea(0, 1)

    let countAsync = 0
    const startAsync = Date.now()
    const TIMEOUT_ASYNC = 5000
    const endAsync = startAsync + TIMEOUT_ASYNC

    while (Date.now() < endAsync) {
        await worker.readMemoryArea(0, 1)
        countAsync++
    }

    const durationAsync = (Date.now() - startAsync) / 1000
    const rateAsync = countAsync / durationAsync

    
    console.log(`    Time: ${durationAsync.toFixed(3)}s`);
    console.log(`    Reads: ${countAsync}`);
    console.log(`    Rate: ${rateAsync.toFixed(0)} reads/sec`);

    // ---------------------------------------------------------
    // 1b. Benchmark Protocol Latency (Command Protocol)
    // ---------------------------------------------------------
    console.log('\n[1b] Benchmarking Protocol Latency (Zero-payload Ping - Command Protocol)...');
    
    const startPingPost = Date.now();
    let countPingPost = 0;
    const TIMEOUT_PING_POST = 5000;
    const endPingPost = startPingPost + TIMEOUT_PING_POST;
    
    while(Date.now() < endPingPost) {
        await worker.writeMemoryArea(0, []);
        countPingPost++;
    }
    const durationPingPost = (Date.now() - startPingPost) / 1000;
    const ratePingPost = countPingPost / durationPingPost;
    
    console.log(`    Time: ${durationPingPost.toFixed(3)}s`);
    console.log(`    Round-Trips: ${countPingPost}`);
    console.log(`    Rate: ${ratePingPost.toFixed(0)} ops/sec`);
    
    // ---------------------------------------------------------
    // 2. Benchmark Shared Memory Read
    // ---------------------------------------------------------
    console.log('\n[2] Benchmarking Shared Memory Read (Direct SAB Access)...')

    const ok = await worker.enableSharedMemory()
    if (!ok) {
        console.error('Shared Memory not available!')
        process.exit(1)
    }
    worker.setSharedMode('run')

    // Let it spin up
    await sleep(100)

    const sharedOutputs = worker.sharedOutputs
    if (!sharedOutputs) throw new Error('Shared Outputs not initialized')

    // We loop for a fixed time because it's so fast
    const durationShared = 5000 // 5s
    let countShared = 0
    const startShared = Date.now()
    const endShared = startShared + durationShared

    // Tight loop reading a byte
    while (Date.now() < endShared) {
        // Simulating reading a specific output
        if (sharedOutputs[0] > 255) {
        } // dummy check to prevent optimization removal (unlikely in JS anyway)
        countShared++
    }

    const actualDurationShared = (Date.now() - startShared) / 1000
    const rateShared = countShared / actualDurationShared

    console.log(`    Time: ${actualDurationShared.toFixed(3)}s`)
    console.log(`    Reads: ${countShared.toLocaleString()}`)
    console.log(`    Rate: ${rateShared.toLocaleString()} reads/sec`)

    // ---------------------------------------------------------
    // 3. Comparison
    // ---------------------------------------------------------
    const speedup = rateShared / rateAsync
    console.log('\n[3] Result')
    console.log(`    Shared Memory is ${speedup.toFixed(1)}x faster than Command Protocol.`)

    // ---------------------------------------------------------
    // 4. Benchmark Memory Write
    // ---------------------------------------------------------
    console.log('\n----------------------------------------------------------------------');
    console.log(':: VovkPLC Shared Memory Write Benchmark');
    console.log('----------------------------------------------------------------------');
    
    // 4a. Async Write
    console.log('\n[4a] Standard Async Write (Command Protocol)...');
    console.log('     (Max 5 seconds)');
    
    const startWriteAsync = Date.now();
    let countWriteAsync = 0;
    const TIMEOUT_WRITE = 5000;
    const endWriteAsync = startWriteAsync + TIMEOUT_WRITE;

    while(Date.now() < endWriteAsync) {
        await worker.writeMemoryArea(0, [1]);
        countWriteAsync++;
    }
    const durationWriteAsync = (Date.now() - startWriteAsync) / 1000;
    const rateWriteAsync = countWriteAsync / durationWriteAsync;
    
    console.log(`    Time: ${durationWriteAsync.toFixed(3)}s`);
    console.log(`    Writes: ${countWriteAsync}`);
    console.log(`    Rate: ${rateWriteAsync.toFixed(0)} writes/sec`);

    // 4b. Shared Write
    console.log('\n[4b] Shared Memory Write (Direct SAB Access)...');
    
    const sharedInputs = worker.sharedInputs;
    if (!sharedInputs) throw new Error('Shared Inputs not initialized');
    
    const startWriteShared = Date.now();
    let countWriteShared = 0;
    const TIMEOUT_WRITE_SHARED = 5000;
    const endWriteShared = startWriteShared + TIMEOUT_WRITE_SHARED;
    
    while(Date.now() < endWriteShared) {
        sharedInputs[0] = (countWriteShared & 0xFF);
        countWriteShared++;
    }
    const durationWriteShared = (Date.now() - startWriteShared) / 1000;
    const rateWriteShared = countWriteShared / durationWriteShared;
    
    console.log(`    Time: ${durationWriteShared.toFixed(3)}s`);
    console.log(`    Writes: ${countWriteShared.toLocaleString()}`);
    console.log(`    Rate: ${rateWriteShared.toLocaleString()} writes/sec`);
    
    console.log(`\n    Shared Write is ${(rateWriteShared/rateWriteAsync).toFixed(1)}x faster.`);

    // ---------------------------------------------------------
    // 5. Benchmark Compilation
    // ---------------------------------------------------------
    console.log('\n----------------------------------------------------------------------');
    console.log(':: VovkPLC Compilation Benchmark');
    console.log('----------------------------------------------------------------------');
    
    const asmLine = 'ptr.const 0\nu8.const 0\nu8.move\n';
    const lines = 1000;
    const largeAsm = 'const val = 0\n' + asmLine.repeat(lines);
    
    console.log(`\n[5] Compiling ${lines} blocks (3000 instructions)...`);
    
    // Run once to warm up (with valid asm)
    await worker.compile('const v = 0\nptr.const 0\nu8.const 0\nu8.move');

    const startCompile = Date.now();
    await worker.compile(largeAsm);
    const durationCompile = (Date.now() - startCompile) / 1000;
    
    console.log(`    Time: ${durationCompile.toFixed(3)}s`);
    console.log(`    Speed: ${(lines * 3 / durationCompile).toFixed(0)} lines/sec`);


    // ---------------------------------------------------------
    // 6. Benchmark Command Protocol (Ring Buffer)
    // ---------------------------------------------------------
    console.log('\n----------------------------------------------------------------------');
    console.log(':: VovkPLC Command Protocol Benchmark (Ring Buffer)');
    console.log('----------------------------------------------------------------------');
    
    console.log('\n[6a] Protocol Latency (Zero-payload Ping - RingBuffer)...');
    
    const startPingRb = Date.now();
    let countPingRb = 0;
    const TIMEOUT_PING_RB = 5000;
    const endPingRb = startPingRb + TIMEOUT_PING_RB;
    
    while(Date.now() < endPingRb) {
        await worker.writeMemoryArea(0, []);
        countPingRb++;
    }
    const durationPingRb = (Date.now() - startPingRb) / 1000;
    const ratePingRb = countPingRb / durationPingRb;
    
    console.log(`    Time: ${durationPingRb.toFixed(3)}s`);
    console.log(`    Round-Trips: ${countPingRb}`);
    console.log(`    Rate: ${ratePingRb.toFixed(0)} ops/sec`);
    console.log(`    Latency: ${(1000/ratePingRb).toFixed(3)} ms/op`);
    
    console.log(`\n    Protocol Speedup: ${(ratePingRb/ratePingPost).toFixed(1)}x faster`);

    // Test Throughput (Pipelined)
    console.log('\n[6b] Protocol Throughput (Pipelined - Command Protocol)...');
    
    const BATCH_SIZE = 100;
    const TOTAL_BATCHES = 200; 
    const TIMEOUT_PIPE = 5000;
    
    const startPipe = Date.now();
    let countPipe = 0;
    
    for (let b = 0; b < TOTAL_BATCHES; b++) {
        if (Date.now() - startPipe > TIMEOUT_PIPE) break;
        const promises = [];
        for (let i = 0; i < BATCH_SIZE; i++) {
             promises.push(worker.writeMemoryArea(0, []));
        }
        await Promise.all(promises);
        countPipe += BATCH_SIZE;
    }
    
    const durationPipe = (Date.now() - startPipe) / 1000;
    const ratePipe = countPipe / durationPipe;
    
    console.log(`    Time: ${durationPipe.toFixed(3)}s`);
    console.log(`    Ops: ${countPipe}`);
    console.log(`    Rate: ${ratePipe.toFixed(0)} ops/sec`);
    console.log(`    Pipeline Latency: ${(1000/ratePipe).toFixed(3)} ms/op`);

    worker.setSharedMode('stop')
    await worker.terminate()
}

benchmark().catch(console.error)
