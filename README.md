# work_reporter

Web service for creating and storing daily and monthly work reports.

Currently only the backend exists: a REST API written in C++20 with
[cpp-httplib](https://github.com/yhirose/cpp-httplib), SQLite (via SQLiteCpp)
for storage, JWT (via jwt-cpp) for authentication, and OpenSSL (PBKDF2-HMAC-SHA256)
for password hashing.

## Features

- User registration and login, returning a signed JWT (`/api/auth/register`, `/api/auth/login`)
- CRUD for work reports, scoped to the authenticated user (`/api/reports`)
- SQLite database with WAL mode, schema created automatically on startup

## Running locally

```bash
cd backend

# install dependencies
conan install . -r conancenter --output-folder=build -s build_type=Release \
  -c tools.cmake.cmaketoolchain:user_presets= --build=missing

# configure and build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# run
DB_PATH=/tmp/work_reporter.db JWT_SECRET=dev-secret PORT=8080 ./build/work_reporter_api
```

The server listens on `PORT` (default `8080`). Configuration is read from
environment variables, see [backend/src/Config.h](backend/src/Config.h):

| Variable     | Default                       | Description                  |
|--------------|--------------------------------|-------------------------------|
| `DB_PATH`    | `data/work_reporter.db`        | Path to the SQLite database file |
| `JWT_SECRET` | `dev-secret-change-me`         | HMAC secret used to sign/verify JWTs |
| `PORT`       | `8080`                         | HTTP listen port             |

## Running with Docker

```bash
cd backend

# build the image
docker build -t work-reporter-api .

# run it, with a named volume so the SQLite file persists across restarts
docker run --rm -p 8080:8080 \
  -e JWT_SECRET=dev-secret-change-me \
  -v work_reporter_data:/app/data \
  --name work-reporter-api \
  work-reporter-api
```

Always override `JWT_SECRET` with your own value outside of local
development — the image ships a placeholder default.

Stop the container with `docker stop work-reporter-api`; the named volume
survives so report data isn't lost (`docker volume rm work_reporter_data` to
wipe it).

### Try it

```bash
# register a user and capture the JWT
TOKEN=$(curl -s -X POST http://localhost:8080/api/auth/register \
  -H 'Content-Type: application/json' \
  -d '{"email":"you@example.com","password":"secret123"}' | python3 -c "import sys,json;print(json.load(sys.stdin)['token'])")

# create a report
curl -s -X POST http://localhost:8080/api/reports \
  -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' \
  -d '{"report_date":"2026-07-09","start_time":"09:00","end_time":"17:00","break_minutes":30,"summary":"did stuff"}'

# list your reports
curl -s http://localhost:8080/api/reports -H "Authorization: Bearer $TOKEN"
```