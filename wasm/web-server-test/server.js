// @ts-check
"use strict"

const express = require("express")
const app = express()
const port = 8080

// Give access to the folder: '../' and './'
app.use(express.static("../"))
app.use(express.static("./"))
app.use(express.static("./src"))

app.get("/", (req, res) => {
    res.sendFile("index.html", { root: "../" })
})

app.listen(port, () => {
    console.log(`Example app listening at http://localhost:${port}`)
})
