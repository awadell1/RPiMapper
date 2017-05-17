function [TestResults] = checkCode(IncludeFiles, SupportFiles, TestScripts, buildDir)
% Function that will move all IncludeFiles and SupportFiles to a new folder, run the provided TestScripts.
% The results of the tests will be reported
% A zip folder will be created containing only the IncludeFiles
% SuportFiles are only used for the TestScripts

% Refresh the Build Directory
[~] = rmdir(buildDir);
mkdir(buildDir)

% Copy all Code Files, Support Files and Test to subdirectory
reqFiles = [IncludeFiles, SupportFiles, TestScripts];
for i = 1:length(reqFiles)
	try
		copyfile(reqFiles{i}, fullfile(buildDir, reqFiles{i}))
	catch
		error('Failed to copy %s', reqFiles{i})
	end
end

% Zip IncludeFiles
zip(buildDir, IncludeFiles);

%% Run Test
workDir = cd;
cd(buildDir)

%Purge Envirorment
close all

% Run Tests
TestResults = runtests(TestScripts);

% Return Home
cd(workDir)
[~] = rmdir(buildDir, 's');






