FROM node:alpine

RUN mkdir -p /home/node/app/node_modules && chown -R node:node /home/node/app

WORKDIR /home/node/app

COPY --chown=node:node src/ ./

COPY --chown=node:node src/static/ ./static/

USER node

RUN npm install

EXPOSE 6101

CMD [ "node", "api.js" ]