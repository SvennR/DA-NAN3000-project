FROM node:13.10.1-buster-slim

RUN apt-get update && \
    apt-get install -y python && \
    apt-get install -y build-essential

RUN mkdir -p /home/node/app/node_modules && chown -R node:node /home/node/app

WORKDIR /home/node/app

COPY --chown=node:node src/ ./

COPY --chown=node:node src/static/ ./static/

USER node

RUN npm install bcrypt \
    cookie-parser \
    express \
    sqlite3

EXPOSE 6101

CMD [ "node", "api.js" ]
