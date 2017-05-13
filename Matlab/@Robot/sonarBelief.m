function [belief] = sonarBelief(robot, pose, sonar)
% sonarBelief: Computes the probability of sonar readings given a pose
%
%   INPUTS
%       robot       A Robot Object
%       robotPose   3-by-1 pose vector in global coordinates (x,y,theta)
%		sonar		3-by-1 list of sonar measurement
%   OUTPUTS
%       belief       The probability of the sonar readings given the pose
%
%   Cornell University
%   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
%   Homework 4
%   Wadell, Alexius

%Predict the sonar readings given the current pose
mu = robot.sonarPredict(pose);

%% Tunning Parameters for sonar model
p_rand = robot.sonar_random;    					% Probability of a random measurement
z_std = robot.sonar_std ^ (1/robot.belief_trust);	% Standard Deviation of Sensor Measurement

% Expand Sonar
sonar = repmat(sonar, size(mu, 1), 1);

% Compute P(Sonar |Map)
del = mu - sonar;
sonarBel = p_rand + (1-p_rand) * exp(-(del.^ 2)/(0.5 * z_std));

% Remove Nans
sonarBel(isnan(sonarBel)) = 1;

% P(Sonar | pose)
belief = prod(sonarBel, 2);
end