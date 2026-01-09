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

/** @type { { address: string, first_visit: Date, last_visit: Date, visits: number, pages: { url: string, count: number }[], blacklisted: boolean }[] } */
const unique_users = []
const get_unique_user = (req) => {
    let address = req.connection.remoteAddress || req.socket.remoteAddress || req.connection.socket.remoteAddress || req.ip
    address = address.replace("::ffff:", "").replace("::1", "").trim() || 'unknown'
    const exists = unique_users.find(user => user.address === address)
    if (exists) return exists
    const new_user = {
        address,
        first_visit: new Date(),
        last_visit: new Date(),
        visits: 1,
        pages: [],
        blacklisted: false
    }
    unique_users.push(new_user)
    return new_user
}
const log_user_interaction = (req, res, user) => {
    const url = req.url
    const now = new Date()
    const blacklist = url.includes("admin") || url.includes("secret") || url.includes("login")
    if (user) {
        user.last_visit = now
        user.visits++
        if (!user.blacklisted && blacklist) {
            user.blacklisted = true
            console.log(`User ${user.address} was blacklisted for visit (${user.visits}) -> "${url}"`)
            return res.status(404).end()
        }
        const existing_page = user.pages.find(page => page.url === req.url)
        if (existing_page) {
            existing_page.count++
        } else {
            user.pages.push({ url: req.url, count: 1 })
        }
        const page = user.pages.find(page => page.url === req.url)
        console.log(`User ${user.address} visit (${user.visits}) -> "${page?.url || '???'}" for the ${page?.count} time.`)
    }
}

app.use((req, res, next) => {
    // check if the request is a favicon request
    if (req.url === "/favicon.ico") {
        res.status(404).end()
        return
    }
    // Log the user interaction if the request is a html page
    // console.log(`Request URL: ${req.url}`)
    const user = get_unique_user(req)
    if (user.blacklisted) {
        console.log(`User ${user.address} is blacklisted, trying to access: ${req.url}`)
        res.status(404).end()
        return
    }
    if (req.url.endsWith(".html") || req.url.endsWith("/")) {
        log_user_interaction(req, res, user)
    }
    next()
})

// Give access to the folder: '../' and './'
app.use(express.static("../"))
app.use(express.static("./"))
app.use(express.static("./src"))
app.use(express.static("./dist"))

app.get("/", (req, res) => {
    res.sendFile("ladder.html", { root: "./" })
})

app.get("/custom/unique_users", (req, res) => {
    res.json(unique_users)
})

// Default 404
app.use((req, res) => {
    res.status(404).end()
})

app.listen(port, () => {
    console.log(`Example app listening at http://localhost:${port}`)
})