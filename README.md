# RPiMapper
Welcome to the GitHub Repo for the RPiMapper Project. Here's a quick run down of where everything is and how to get started.

| Folder        | Contents                                                      |
|---------------|---------------------------------------------------------------|
| Matlab        | Contains the code that runs on the host computer              |
| docs          | Project website files                                         |
| socketDemo    | Quick demo showing socket communications between two computer |
| socket_stream | Contains the code that runs on the Raspberry Pi               |
| sonarMain     | Contains the code that runs on the Arduino                    |

## Getting Started #
On the Raspberry Pi
1. Clone the repo `https://github.com/awadell1/RPiMapper.git`
2. cd into `/socke_stream`
3. Run `bash compile.sh` to compile the source code with `gcc`
4. Start the control program with `./socketStream`

On the host computer:
1. Install MATLAB (Developed on 2016a) and Arduino IDE
2. Clone the repo `https://github.com/awadell1/RPiMapper.git`
3. Using the Arduino IDE write sonarMain.ino (It's in `/sonarMain`) to the Arduino nano.
4. Open the `/Matlab` directory in MATLAB
5. Connect to the robot using `robot = Robot(ROBOT IP ADDRESS HERE)`
6. Enter `freedriveProgram` into the console to start the mapping program
7. Use your arrow keys to move around and watch as RPiMapper maps the envirorment

You should now be able to drive RPiMapper and while mapping your envirorment.

