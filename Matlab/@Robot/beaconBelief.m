function [belief] = beaconBelief(robot, pose, beacon)
% sensorBelief: Computes the probability of the beacon reading given a pose
%
%   INPUTS
%       robot       A Robot Object
%       robotPose   n-by-3 pose vector in global coordinates (x,y,theta)
%       beacon      The beacon readings
%   OUTPUTS
%       belief      The probability of the beacon readings given the pose

% Predict Beacon
muBeacon = robot.predictBeacon(pose);

% Drop Yvec and RotVect from beacon Reading
% Camera Zvec -> dx Camera Xvec -> dy
beacon = beacon(:, [1, 4, 2]);

% Drop Unseen Beacons
muBeacon = muBeacon(:,:,beacon(:,1));

% Compute del
dev = muBeacon - repmat(reshape(beacon', 1, 3, []),size(pose,1),1);
dev(:,1,:) = []; %Drop Beacon ids

% Compute Probability
z_std = robot.camera_std ^ (1/robot.belief_trust);
belief3d = robot.camera_random +...
    (1- robot.camera_random) .* exp(-dev.^2 /(0.5*z_std));

% Compute Belieft 
belief = prod(prod(belief3d,2),3);
end