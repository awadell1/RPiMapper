function beacons = ReadBeacon(beaconPort)
%READBEACON  Retrieve the most recent ARtag reading from a Beagleboard.
%   ReadBeacon(beaconPort) returns
%   beacons which is [id x y z yaw]
%   
%   id = The id of the beacon
%   x = The x position of the beacon
%   y = The y position of the beacon
%   z = The z position of the beacon
%   yaw = The orientation of the beacon
%
%   The total number of beacon detected is size(beacons,1) and it should
%   not be more than 10.
%   
%   When there is no ARtag detected, isempty(beacons) return 1.
%
%   The udp port object 'beaconPort' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
%   NOTE: If a beacon requested is not seen in the view then values of NaN 
%   are substituted for real data.
%
% By: Chuck Yang, ty244, 2012

warning off all;

global BEACON_OFFSET;
if isempty(BEACON_OFFSET)
BEACON_OFFSET = 0;
end

% the maximum number of ARtags that will get detected in one camera view.
MAXARTAGSEEN = 10;
HEADER = 12;
% Initialize preliminary return value
beacons = [];
try
    fclose(beaconPort);
    pause(.01);
    fopen(beaconPort);

    %read in packet and get size
    [packet size] = fread(beaconPort);
    if size > 15
        for i = 1:MAXARTAGSEEN
            idIndex = HEADER+(i-1)*4+1;
            id = typecast(uint8(packet(idIndex:idIndex+3)),'int32');
            id = typecast(double(id), 'double');
            if id == -1
                break;
            end
            xIndex = idIndex+4*MAXARTAGSEEN;
            x = typecast(uint8(packet(xIndex:xIndex+3)),'single');
            x = typecast(double(x), 'double');
            yIndex = xIndex+4*MAXARTAGSEEN;
            y = typecast(uint8(packet(yIndex:yIndex+3)),'single');
            y = typecast(double(y), 'double');
            zIndex = yIndex+4*MAXARTAGSEEN;
            z = typecast(uint8(packet(zIndex:zIndex+3)),'single') + BEACON_OFFSET;
            z = typecast(double(z), 'double');
            yawIndex = zIndex+4*MAXARTAGSEEN;
            yaw = typecast(uint8(packet(yawIndex:yawIndex+3)),'single');
            yaw = typecast(double(yaw), 'double');

            beacons = [beacons; id x y z yaw];
        end
    end
catch
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
end

end