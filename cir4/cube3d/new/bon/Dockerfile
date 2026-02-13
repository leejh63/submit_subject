FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 필요한 패키지 설치 (MLX + X11 + 빌드툴)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential make gcc \
    pkg-config \
    libx11-dev libxext-dev libbsd-dev \
    libxrandr-dev libxrender-dev libxfixes-dev libxi-dev \
    libgl1-mesa-dev mesa-utils \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 소스 복사
COPY . .

# 빌드
RUN make -j

# 기본 실행 (valid1.cub)
CMD ["bash", "-lc", "./cub3d maps/valid1.cub"]

