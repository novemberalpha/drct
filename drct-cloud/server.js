const { prisma } = require('./generated/prisma-client')
const express = require('express')
const bodyParser = require('body-parser')
const app = express()

app.use(bodyParser.json())

app.get(`/sensorPayloads`, async (req, res) => {
  const sensorPayloads = await prisma.sensorPayloads()
  res.json(sensorPayloads)
})

app.get('/sensorPayload/:payloadId', async (req, res) => {
  const { payloadId } = req.params
  const sensorPayload = await prisma.sensorPayload({ id: payloadId })
  res.json(sensorPayload)
})

// app.get('/posts/user/:userId', async (req, res) => {
//   const { userId } = req.params
//   const postsByUser = await prisma.user({ id: userId }).posts()
//   res.json(postsByUser)
// })

app.post('/sensorPayload', async (req, res) => {
  const newSensorPayload = await prisma.createSensorPayload(req.body)
  res.json(newSensorPayload)
})

// app.post('/post/draft', async (req, res) => {
//   const newPost = await prisma.createPost(req.body)
//   res.json(newPost)
// })

app.put(`/sensorPayload/:payloadId`, async (req, res) => {
  const { payloadId } = req.params
  const body = req.body
  const updatedSensorPayload = await prisma.updateSensorPayload({
    where: { id: payloadId },
    data: body,
  })
  res.json(updatedSensorPayload)
})

app.listen(8080, () =>
  console.log('Server is running on http://localhost:8080'),
)
