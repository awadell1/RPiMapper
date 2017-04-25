function [beacon] = predictBeacon(robot, pose)
% predictBeacon: Predicts the current beacon readings for the given pose
%   INPUT
%       robot   A Robot Object
%       pose    A n-by-3 list of robot poses [x, y, theta]
%   OUTPUT
%       beaconStruct    A n-by-3-by-k beacon cell array containing [tag, dx, dy]
%                       Where k is the number of beacons on the map

% Get a list of beacons
mapBeacon = robot.map.beacon;
nBeacon = size(mapBeacon, 1);

% Number of poses to check
nPose = size(pose, 1);

% Reshape into 3D matrix
beaconG = repmat(reshape(mapBeacon', 1, 3, []), nPose, 1);

% Expand pose
pos_3d = repmat(pose(:, 1:2), 1, 1, nBeacon);
theta_3d = repmat(pose(:,3), 1, 1, nBeacon);

% Compute dx, dy
beaconG(:, 2:3, :) = beaconG(:, 2:3, :) - pos_3d;

% Rotate into local frame
beacon = beaconG;
beacon(:,2,:) = sum(beaconG(:,2:3,:) .* [cos(theta_3d) sin(theta_3d)], 2);
beacon(:,3,:) = -sum(beaconG(:,2:3,:) .* [-sin(theta_3d) cos(theta_3d)], 2);

% Account for Camera Radius
beacon(:,2,:) = beacon(:,2,:) - robot.camera_radius;
end



