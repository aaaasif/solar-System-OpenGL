Abdullah Al Asif
2022521460130

---

# 3D Solar System with Moons and Milky Way Background

This project is a 3D visualization of the solar system, featuring the Sun, eight planets, their respective moons, and a background of the Milky Way galaxy. It uses OpenGL for rendering and GLUT for window management, camera control, and event handling.

## Features

- **Sun and Planets**: Realistic rendering of the Sun and its orbiting planets with textures.
- **Moons**: Each planet (except Mercury) has a moon that orbits around it, with proper inclination and rotation.
- **Milky Way Background**: A textured background representing the Milky Way galaxy.
- **Camera Control**: Use the mouse and keyboard to control the camera and zoom in/out of the solar system.
- **Planetary Orbits**: Each planet orbits the Sun with different speeds, and the moons orbit their respective planets.
- **Textures**: Textures are applied to the Sun, planets, moons, and background.

## Requirements

- OpenGL
- GLUT (FreeGLUT)
- C++ Compiler (e.g., GCC or MSVC)

### Libraries:
- **OpenGL**: For rendering the solar system and background.
- **GLUT**: For handling window management, input events, and animation.

### Textures:
- Sun, planets, and moons have texture files in `.bmp` format.
- Milky Way background also uses a `.bmp` texture.

## Installation

### Step 1: Install Dependencies

Ensure that you have OpenGL and GLUT installed on your system.

- **Windows**: Use freeglut or the default GLUT library.
- **Linux**: Install `freeglut` via your package manager (e.g., `sudo apt-get install freeglut3 freeglut3-dev`).
- **macOS**: GLUT should be pre-installed with Xcode command line tools.

### Step 2: Clone or Download the Repository

```bash
git clone https://github.com/yourusername/3d-solar-system.git
cd 3d-solar-system
```

### Step 3: Compile the Code

To compile the program, you can use a C++ compiler. Here's an example using `g++` (ensure you have OpenGL and GLUT development libraries installed):

```bash
g++ -o solar_system main.cpp -lGL -lGLU -lglut
```

### Step 4: Run the Program

After compiling, run the executable:

```bash
./solar_system
```

This will open a window with a 3D rendering of the solar system.

## Controls

### Mouse Controls:
- **Left Click & Drag**: Rotate the camera around the solar system.
- **Mouse Wheel**: Zoom in and out of the solar system (limited zoom range).
  
### Keyboard Controls:
- **W**: Pan the camera up.
- **S**: Pan the camera down.
- **A**: Pan the camera left.
- **D**: Pan the camera right.
- **R**: Reset the camera to its default position.

## Features in Detail

- **Planet and Moon Texturing**: All planets and moons are textured using 24-bit BMP images. The textures are loaded from files in the `texture/` directory.
- **Orbiting Planets and Moons**: The planets orbit the Sun, and each moon orbits its respective planet. The speed of the orbit is based on the planet’s position in the solar system.
- **Background Texture**: A large sphere represents the Milky Way galaxy, providing a background for the solar system. The texture is applied to this sphere.
- **Lighting**: The Sun acts as the light source for the solar system, casting light on the planets and their moons.

## File Structure

```
3d-solar-system/
├── main.cpp                # Source code for the solar system simulation
├── texture/                # Directory containing texture files
│   ├── sun.bmp             # Texture for the Sun
│   ├── mercury.bmp         # Texture for Mercury
│   ├── venus.bmp           # Texture for Venus
│   ├── earth.bmp           # Texture for Earth
│   ├── mars.bmp            # Texture for Mars
│   ├── jupiter.bmp         # Texture for Jupiter
│   ├── saturn.bmp          # Texture for Saturn
│   ├── uranus.bmp          # Texture for Uranus
│   ├── neptune.bmp         # Texture for Neptune
│   ├── pluto.bmp           # Texture for Pluto
│   ├── moon.bmp            # Texture for moons
│   └── milkyway.bmp        # Background texture (Milky Way)
└── README.md               # This file
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- OpenGL and GLUT for rendering and window management.
- The textures used in the project are publicly available images or created by the project author.

---

This README provides an overview of the project, setup instructions, and details about how to use and control the program. You can modify or expand the sections based on additional features or customization you might want to include.