classdef gridData
	% gridData: Class for working with data on a grid
	%
	%	Cornell University
	%	MAE 4180/5180 CS 3758: Autonomous Mobile Robots
	%	Homework 5
	%	Wadell, Alexius
	
	properties
		data = [];		% Stores grid data
		edgeX = [];		% X-coordinates of the grid edges
		edgeY = [];		% Y-cood of the grid edges
	end
	
	properties(Dependent)
		cellX	% X-cordinate of the cell center
		cellY	% Y-cordinate of the cell center
	end
	methods
		function obj = gridData(edgeX, edgeY, cellData)
			% gridData: Constructor for the gridData class
			%
			%	INPUTS
			%		edgeX		The X-coordinates of the edges of the cells
			%		edgeY		The Y-coordinates of the edges of the cells
			%		cellData	The data to store in gridData (Defaults to zeros)
			%					If single value -> All cells set to that value
			%					If matrix -> cells set to that matrix
			%	OUTPUTS
			%		obj			A gridData object
			%
			%	Cornell University
			%	MAE 4180/5180 CS 3758: Autonomous Mobile Robots
			%	Homework 5
			%	Wadell, Alexius
			
			% Set the edges for the grid
			obj.edgeX = edgeX;
			obj.edgeY = edgeY;
			
			% Check if cell data was set
			switch nargin
				case 2
					% Set cell data to zeros
					obj.data = zeros(length(edgeX) - 1, length(edgeY) -1);
				case 3
					% Check if matrix or scalar
					if all(size(cellData) == [length(edgeX)-1, length(edgeY)-1])
						% Assign cellData to data
						obj.data = cellData;
					elseif isscalar(cellData)
						% Set all values to cellData
						obj.data = cellData .* ones(length(edgeX) - 1, length(edgeY) -1);
					else
						% cellData is not of the correct size
						error('Incorrect cellDataSize')
					end
				otherwise
					error('Invaid number of inputs')
			end
		end
		function [X, Y] = cellCenter(obj)
			% cellCenter: Returns the coordinats of each cell's center as a meshgrid
			%
			%	INPUTS
			%		obj A gridData object
			%
			%	OUTPUTS
			%		X	The x-coordinates of the cell centers
			%		Y	The y-soordinates of the cell centers
			%
			%	Cornell University
			%	MAE 4180/5180 CS 3758: Autonomous Mobile Robots
			%	Homework 5
			%	Wadell, Alexius
			
			% Return the x and y coordinates of each cells center
			[X, Y] = meshgrid(obj.cellX, obj.cellY);
			X = X'; Y = Y';
		end
		function [X,Y] = cellCorners(obj)
			% cellCorners: Returns the coordinats of the cell corners as a meshgrid
			%
			%	INPUTS
			%		 obj A gridData object
			%
			%	OUTPUTS
			%		 X	The x-coordinates of the cell corners
			%		 Y	The y-soordinates of the cell corners
			%
			%	Cornell University
			%	MAE 4180/5180 CS 3758: Autonomous Mobile Robots
			%	Homework 5
			%	Wadell, Alexius
			
			% Return the x and y coordinates of each cells corners
			[X, Y] = meshgrid(obj.edgeX, obj.edgeY);
			X = X'; Y = Y';
		end
		function [i,j] = hasPoint(obj, x, y)
			% hasPoint: Return the indices of the cells containg the given points
			%
			%	INPUTS
			%		obj A gridData object
			%		x	The x cordinates of the points to test (scalar or vector)
			%		 y	The y cordinates of the points to test (scalar or vector)
			%
			%	OUTPUTS
			%		i	The row index of the cell containg the point
			%		j	The column index of the cell containing the point
			%
			%	Note: [i(p), j(p)] gives the index of point [x(p), y(p)]
			%
			%	Cornell University
			%	MAE 4180/5180 CS 3758: Autonomous Mobile Robots
			%	Homework 5
			%	Wadell, Alexius
			
			% Checks if a given point is within a cell
			lX = obj.edgeX(1:end-1);  uX = obj.edgeX(2:end);
			lY = obj.edgeY(1:end-1);  uY = obj.edgeY(2:end);
			
			% Expand lX and lY
			lX = repmat(lX, [length(x), 1]);
			lY = repmat(lY, [length(x), 1]);
			
			% Check for point on the interior of cells
			i = nan(length(x), 1); j =i;
			[p, k] = find(x >= lX & x < uX);
			i(p) = k;
			[p, k] = find(y >= lY & y < uY);
			j(p) = k;
			
			% Remove nan
			valid = ~isnan(i) & ~isnan(j);
			i = i(valid); j = j(valid);
		end
		
		%% Access Methods
		function cellX = get.cellX(obj)
			% get.cellX: Return the x coordinates of the cell centers
			%
			%	INPUTS
			%		obj A gridData object
			%
			%	OUTPUTS
			%		cellX	The x-coordinates of the cell centers
			%
			%	Cornell University
			%	MAE 4180/5180 CS 3758: Autonomous Mobile Robots
			%	Homework 5
			%	Wadell, Alexius
			
			cellX = obj.edgeX(1:end-1) + diff(obj.edgeX)/2;
		end
		function cellY = get.cellY(obj)
			% get.cellY: Return the y coordinates of the cell centers
			%
			%	INPUTS
			%		obj A gridData object
			%
			%	OUTPUTS
			%		cellX	The y-coordinates of the cell centers
			%
			%	Cornell University
			%	MAE 4180/5180 CS 3758: Autonomous Mobile Robots
			%	Homework 5
			%	Wadell, Alexius
			cellY = obj.edgeY(1:end-1) + diff(obj.edgeY)/2;
		end
		
		%% Externally Defined Functions
		[inBeam, cellRange] = hasBeam(g,pose, beamWidth)
		
		varargout = plot(obj, varargin)
	end
	
end

