function [V, w] = feedbacklin(robot, Vx, Vy, pose)
% feedbacklin: Linearizes the command [Vx, Vy] -> [V, w]
% 
%   [V, w] = feedbacklin(Vx, Vy, pose, e)
% 
%   INPUTS
%		robot	Robot Object
%       Vx      Desired x velocity (Global)
%       Vy      Desired y velocity (Global)
%       pose    Pose of Create Robot [x, y, theta]
%
%   OUTPUTS
%       V       Create Forward Velocity (Local)
%       w       Create Angular Velocity (Local)

% 
%   NOTE: Assume differential-drive robot whose wheels turn at a constant 
%         rate between sensor readings.
% 
%   Cornell University
%   MAE 5180: Autonomous Mobile Robots
%   Alexius Wadell

%Represent Vx, Vy in the robot's local coordinates
R = [ cos(pose(3))  sin(pose(3));...
     -sin(pose(3))  cos(pose(3))];
x = R * [Vx; Vy];

%Set the forward velocity to the projection of (Vx, Vy) onto the local x-axis
V = x(1);

%Set the forward velocity to the projection of (Vx, Vy) onto the local y-axis
%Scale by 1/e for tuning purposes
w = x(2)/robot.drive_epsilon;