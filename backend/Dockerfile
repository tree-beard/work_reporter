FROM debian:bookworm AS build

# perl is required by OpenSSL's Configure script when Conan builds it from source.
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    perl \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN pip install conan --break-system-packages \
    && conan profile detect --force

WORKDIR /src

# Installed before copying source so this layer (the slow one) only
# re-runs when conanfile.txt actually changes, not on every code edit.
COPY conanfile.txt CMakeLists.txt ./
RUN conan install . -r conancenter --output-folder=build -s build_type=Release \
    -c tools.cmake.cmaketoolchain:user_presets= --build=missing

COPY src ./src
RUN cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel

FROM debian:bookworm-slim AS runtime

RUN useradd --system --create-home --shell /usr/sbin/nologin appuser
WORKDIR /app

COPY --from=build /src/build/work_reporter_api ./work_reporter_api

ENV DB_PATH=/app/data/work_reporter.db
ENV JWT_SECRET=change-me-in-prod
ENV PORT=8080

RUN mkdir -p /app/data && chown -R appuser:appuser /app
USER appuser

EXPOSE 8080
ENTRYPOINT ["./work_reporter_api"]
