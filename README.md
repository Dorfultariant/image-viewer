# iv

Simple SDL3 image viewer.

## Features

- PNG
- JPEG
- Transparency support
- Directory loading
- Absolute paths
- Relative paths
- Wildcards (*.png, *.jpg)
- Multiple files as arguments
- Aspect ratio preserved
- Zoom
- Pan
- Rotation
- Mirroring

## Build

### Debian/Ubuntu

Install dependencies:

```bash
sudo apt install libsdl3-dev libsdl3-image-dev cmake g++
```

Build:

```bash
mkdir -p build
cd build

cmake ..
cmake --build .
```

## Usage

Single image:

```bash
iv image.png
```

Multiple images:

```bash
iv a.png b.jpg c.png
```

Directory:

```bash
iv ./images
```

Wildcard:

```bash
iv *.png
```

Mixed:

```bash
iv ./images *.jpg image.png
```

## Controls

| Key | Action |
|-------|-------|
| Left / h | Previous Image |
| Right / l | Next Image |
| Up / k | Zoom In |
| Down / j | Zoom Out |
| Shift + h/j/k/l | Move Image |
| Shift + Arrows | Move Image |
| Ctrl + Left/h | Rotate Left 90° |
| Ctrl + Right/l | Rotate Right 90° |
| m | Vertical Mirror |
| Shift + m | Horizontal Mirror |
| Esc | Quit |
