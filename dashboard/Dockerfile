# Stage 1: compilazione
FROM node:20 AS build
WORKDIR /app
COPY package.json . package-lock.json ./
RUN npm ci 
COPY . .
RUN npm run build

# Stage 2: immagine finale
FROM  nginx:alpine
COPY --from=build /app/dist /usr/share/nginx/html

 