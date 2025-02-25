// @ts-check
"use strict"

const express = require("express")
const app = express()
const port = 8080


/** @type { (date?: Date | number | undefined) => string } */// 'YYYY-MM-DD 24HH:MM:SS'
const timestamp = (date) => new Date(date || new Date()).toISOString().replace("T", " ").replace("Z", "")
const cl = console.log
// Override console.log to include a timestamp
/** @type { (...args: any[]) => void } */
console.log = (...args) => cl(`[${timestamp()}]:`, ...args)

/** @type { { address: string, first_visit: Date, last_visit: Date, visits: number, pages: { url: string, count: number }[] }[] } */
const unique_users = []
const log_user_interaction = (req) => {
    let address = req.connection.remoteAddress || req.socket.remoteAddress || req.connection.socket.remoteAddress || req.ip
    address = address.replace("::ffff:", "").replace("::1", "").trim() || 'unknown'
    const now = new Date()
    const existing_user = unique_users.find(user => user.address === address)
    if (existing_user) {
        existing_user.last_visit = now
        existing_user.visits++
        const existing_page = existing_user.pages.find(page => page.url === req.url)
        if (existing_page) {
            existing_page.count++
        } else {
            existing_user.pages.push({ url: req.url, count: 1 })
        }
        const page = existing_user.pages.find(page => page.url === req.url)
        console.log(`User ${address} visited "${page?.url || '???'}" for the ${page?.count} time. Total visits: ${existing_user.visits}`)
    } else {
        unique_users.push({
            address,
            first_visit: now,
            last_visit: now,
            visits: 1,
            pages: [{ url: req.url, count: 1 }]
        })
        console.log(`New user ${address} visited "${req.url}" for the first time`)
    }
}

app.use((req, res, next) => {
    // check if the request is a favicon request
    if (req.url === "/favicon.ico") {
        res.status(204).end()
        return
    }
    // Log the user interaction if the request is a html page
    // console.log(`Request URL: ${req.url}`)
    if (req.url.endsWith(".html") || req.url.endsWith("/")) {
        log_user_interaction(req)
    }
    next()
})

// Give access to the folder: '../' and './'
app.use(express.static("../"))
app.use(express.static("./"))
app.use(express.static("./src"))

app.get("/", (req, res) => {
    res.sendFile("index.html", { root: "../" })
})

app.get("/unique_users", (req, res) => {
    res.json(unique_users)
})

app.listen(port, () => {
    console.log(`Example app listening at http://localhost:${port}`)
})
