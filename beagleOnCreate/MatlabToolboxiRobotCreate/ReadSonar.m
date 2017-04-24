function distance = ReadSonar(sonarPort, varargin)
%READSONAR  Retrieve the most recent sonar reading from a Beagleboard.
%   ReadSonar(sonarPort, sonarNum) returns the last sonar distance reading that was 
%   captured by a Beagleboard attached to an iRobot Create or Roomba, in 
%   meters.
%   sonarLeft = distance measurement of sonar at the left.
%   sonarFront = distance measurement of sonar at the front.
%   sonarRight = distance measurement of sonar at the right.
%   sonarBack = distance measurement of sonar at the back (not
%   implemented).
%
%   The udp port object 'sonarPort' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%   
%   NOTE: The first time a new sonar sensor is used, it is recommended 
%   that you calibrate it using the CalibrateSonar function.
%
%   See also CalibrateSonar and TestSonar.
%
% By: Chuck Yang, ty244, 2012

warning off all;

global SONAR_OFFSET;
if isempty(SONAR_OFFSET)
SONAR_OFFSET = [0 0 0];
end

% Initialize preliminary return value
sonarLeft = NaN;
sonarFront = NaN;
sonarRight = NaN;
sonarBack = NaN;
try
    fclose(sonarPort);
    pause(.01);
    fopen(sonarPort);

	[packet size] = fread(sonarPort);
	if size > 14
		sonarLeft = typecast(uint8(packet(13:16)),'single');
		sonarFront = typecast(uint8(packet(17:20)),'single');
		sonarRight = typecast(uint8(packet(21:24)),'single');
        if sonarLeft > 0
            sonarLeft = sonarLeft + SONAR_OFFSET(3);
        else
            sonarLeft = NaN;
        end
        if sonarFront > 0
            sonarFront = sonarFront + SONAR_OFFSET(2);
        else
            sonarFront = NaN;
        end
        if sonarRight > 0
            sonarRight = sonarRight + SONAR_OFFSET(1);
        else
            sonarRight = NaN;
        end
    end
    distSonar = [sonarRight sonarFront sonarLeft sonarBack];
    if isempty(varargin)
        distance = distSonar(2);
    elseif varargin{1} ~= 0
        distance = distSonar(varargin{1});
    else
        distance = distSonar;
    end
catch err
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
    err.identifier
end

end