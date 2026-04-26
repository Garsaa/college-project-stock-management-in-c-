## Features
- Create inventory items.
- List stock movements.
- List items.
- Get an item by code.
- Filter items by code or partial name.
- Update item metadata.
- Register inbound stock.
- Register outbound stock.
- Delete items.
- Keep item movement history in the database.

## Stack
- `C++20`
- `Drogon`
- `PostgreSQL`
- `CMake`
- `MinGW`

## Responsibilities

- `src/main.cpp`
  Starts the application, ensures the schema exists, and boots `Drogon`.
- `src/api/AppConfig.cpp`
  Loads `.env`, reads environment variables, and builds the PostgreSQL configuration.
- `src/api/InventoryService.cpp`
  Holds business rules and SQL access.
- `src/http/InventoryController.cpp`
  Maps HTTP requests and JSON payloads to the service layer.
- `sql/schema.sql`
  Startup schema source used by the application.

## Build
```powershell
cmake -S . -B build-api -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE=C:/codex-vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic `
  -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic

cmake --build build-api -j 8
```

## Run
The API loads a local `.env` file automatically from the project root. Use
`.env.example` as the template and keep the real values in `.env`.

.\build-api\inventory_api.exe

At startup, the API creates the `estoque_items` and
`estoque_inventory_movements` tables and their indexes if they do not exist
yet by executing `sql/schema.sql`.

## Docker
The repository includes a multi-stage `Dockerfile` for Linux environments such
as Render. The application now accepts `PORT` as a fallback to `APP_PORT`,
which helps on platforms that inject the HTTP port automatically.

### Build the image

```powershell
docker build -t inventory-api .
```

### Run the container

Pass the PostgreSQL settings with `--env-file` or individual `-e` flags. The
example below publishes the API on `http://localhost:2020`.

```powershell
docker run --name inventory-api `
  --env-file .env `
  -p 2020:2020 `
  inventory-api
```

If your PostgreSQL instance is running on your host machine instead of inside
Docker, do not keep `POSTGRES_HOST=127.0.0.1` in that env file. From inside the
container, `127.0.0.1` points to the container itself. On Docker Desktop, use
`POSTGRES_HOST=host.docker.internal`. If the database runs in another
container, use the other container's service name on the same Docker network.

If your cloud platform provides `PORT`, the container will bind to that port
automatically. Locally, `APP_PORT` still works the same way as before.

### Monitor the container

```powershell
docker logs -f inventory-api
docker ps
docker inspect --format='{{json .State.Health}}' inventory-api
docker stats inventory-api
```

## Endpoints
### Health

```http
GET /health
```

### List movements

```http
GET /api/movements
```

### List items

```http
GET /api/items
GET /api/items?search=mouse
```

### Get an item by code

```http
GET /api/items/D001
```

### Create an item

```http
POST /api/items
Content-Type: application/json
```

```json
{
  "code": "D001",
  "item_type": "product",
  "name": "Wireless Mouse",
  "description": "Mouse for office use.",
  "information_link": "https://example.com/mouse",
  "quantity": 10,
  "unit_price": 89.9,
  "location": "Shelf A1"
}
```

### Update an item

```http
PATCH /api/items/D001
Content-Type: application/json
```

```json
{
  "description": "Ergonomic wireless mouse.",
  "unit_price": 99.5,
  "location": "Shelf B2"
}
```

### Register inbound stock

```http
POST /api/items/D001/inbound
Content-Type: application/json
```

```json
{
  "quantity": 5,
  "note": "Weekly replenishment."
}
```

### Register outbound stock

```http
POST /api/items/D001/outbound
Content-Type: application/json
```

```json
{
  "quantity": 3,
  "note": "Separated for customer order."
}
```

### Delete an item

```http
DELETE /api/items/D001
```
