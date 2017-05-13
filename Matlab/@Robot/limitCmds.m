function[cmdV,cmdW] = limitCmds(robot, fwdVel,angVel)
% LIMITCMDS: scale forward and angular velocity commands to avoid
% saturating motors.
% 
%   [CMDV,CMDW] = LIMITCMDS(FWDVEL,ANGVEL,MAXV,WHEEL2CENTER) returns the 
%   forward and angular velocity commands to drive a differential-drive 
%   robot whose wheel motors saturate at +/- maxV.
%
%   [CMDV,CMDW] = LIMITCMDS(FWDVEL,ANGVEL) Assumes a maxV of 0.5 m/s and a
%   wheel2center distance of 0.13m
% 
%   INPUTS
%       robot       Robot Object
%       fwdVel      desired forward velocity (m/s)
%       angVel      desired angular velocity (rad/s)
% 
%   OUTPUTS
%       cmdV        scaled forward velocity command (m/s)
%       cmdW        scaled angular velocity command (rad/s)
% 
% 
%   Cornell University
%   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
%   Homework #1
%   Wadell, Alexius


% Compute Wheel Speeds
wR = fwdVel + angVel*robot.wheel2center;
wL = fwdVel - angVel*robot.wheel2center;

%Compute Slowdown Factor
a = robot.maxVelocity ./ abs([wR wL]);
a = min([a 1]); %Only scale command if saturating

%Scale forward and angular velocity commands
cmdV = a * fwdVel;
cmdW = a * angVel;
end
 