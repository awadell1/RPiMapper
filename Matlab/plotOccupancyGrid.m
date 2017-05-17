function varargout = plotOccupancyGrid(MapOcc)
% plotOccupancyGrid: Plots the occupancy grid stored in MapOcc
%	INPUT
%		MapOcc		A gridData object storing the log odd for each cell's occupancy
%	OUTPUT (Passed via varargout)
%		h			An image handle for the plotted log odds


% Create a figure
figure

% Plot Log Ods
h = plot(MapOcc);

% Check if an output was requested
if nargout ==1
	varargout{1} = h;
end
