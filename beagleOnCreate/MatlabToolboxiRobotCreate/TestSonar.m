function TestSonar(ports, testType)
%TESTSONAR  Test the three sonar sensors on a Beagleboard.
%   TestSonar(ports, testType) takes continuous sonar readings and plots them.
%
%   The value of 'testType' determines the type of test:
%
%    - 'stationary' will keep the robot still and plot a distance-vs-time
%      graph.
%    - 'rotate' will spin the robot in place and plot a polar map of its
%      surroundings.
%
%   The test will continue indefinitely, until the user forces the function
%   to quit (by pressing CTRL-C in the console).
%
%   The udp port object 'ports' must first be initialized with the
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
%   See also CalibrateSonar, and ReadSonar.

switch testType
    case 'stationary'
        ROTATE = false;
    case 'rotate'
        ROTATE = true;
    otherwise
        error('Unknown test type.');
end

figure(1), clf;
hold on;

if ROTATE
    orient = 0; % Consider our initial orientation to be an angle of 0
    AngleSensorRoomba(ports.create); % Clear the angle sensor register

    SetFwdVelAngVelCreate(ports.create, 0, 0.1); % Start the robot rotating in place
end

x = 0;

try
    while 1
        if ROTATE
            % Update the orientation based on the angle sensor (dead reckoning!)
            dangle = AngleSensorRoomba(ports.create);
            if ~isempty(dangle)
                orient = orient + dangle;
            else
%                 disp('angle sensor didnt return things');
            end
        end
        
        % Get the sonar reading
        d = ReadSonar(ports);

        % Wait a tic
        pause(0.5)

        % Update the plot
        if ROTATE
%             plot(d.sonar1*cos(orient-pi/4), d.sonar1*sin(orient-pi/4), 'x', 'MarkerSize', 10);
            dx = d.sonar2*cos(orient);
            dy = d.sonar2*sin(orient);
            plot(d.sonar2*cos(orient), d.sonar2*sin(orient), '*', 'MarkerSize', 10);
            
%             plot(d.sonar3*cos(orient+pi/4), d.sonar3*sin(orient+pi/4), 'x', 'MarkerSize', 10);

            ylim([-1, 1])
            xlim([-1, 1])
        else
            plot(x, d.sonar1, 'xr', x, d.sonar2, 'xg', x, d.sonar3, 'xb', 'MarkerSize', 10);
            ylim([0, 3])
            xlim([x-50, x+10])  % Keep a constant window size
        end
        x = x + 1;
        drawnow
    end
catch
    % Stop the robot on error, if necessary
    if ROTATE
        SetFwdVelAngVelCreate(ports.create, 0, 0);
    end
end
