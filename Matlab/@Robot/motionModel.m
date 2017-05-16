function pose = motionModel(robot, pose0, u)
% motionModel: Predicts the pose of the Robot from odometry
%	INPUT
%		robot	A Robot Object
%		pose0	The prior post of the robot n-by-3
%		u		Odometry Measurement 2-by-1
%	OUTPUT
%		pose	The predicted pose of the robot n-by-3

% Number of input poses
nPose = size(pose0, 1);

% Handle empty u
if isempty(u)
	u = [0, 0];
end

% Model Process Noise
u_noise = repmat(u, nPose, 1) + ...
	[robot.fwdVel_std * randn(nPose, 1),...
	 robot.angVel_std * randn(nPose, 1)];

% Predict Pose
pose = robot.intergrateOdom(pose0, u_noise);