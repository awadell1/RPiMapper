% Purge Workspace
clear classes global

% RunCheckCode
buildDir = 'HW5_code';

IncludeFiles = {...
'@Robot',...
'@gridData',...
'@RobotMap',...
'TestOccupancyGrid.m',...
'plotOccupancyGrid.m',...
'logOddsBump.m',...
'logOddsSonar.m',...
'motionControl.m',...
};

SupportFiles = {...
'debugData.mat',...
};

TestScripts = {....
'runTestOccupancyGrid.m'
};


[TestResults] = checkCode(IncludeFiles, SupportFiles, TestScripts, buildDir);