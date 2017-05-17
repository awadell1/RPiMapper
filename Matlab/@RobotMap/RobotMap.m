classdef RobotMap
    % RobotMap: Class for handling map files and general calculations with maps
    %
    %   Cornell University
    %   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
    %   Alexius Wadell
    
    properties
        %Stores Wall Endpoints:[x1, y1, x2, y2]
        wall = [];
        
        % Stores beacon locations: [tagNum, x, y]
        beacon = [];        
    end

    % Define Dependant Properties
    properties(Dependent)
        nWall
    end
    
    methods
        function obj = RobotMap(map)
            % RobotMap: Intalizes the RobotMap object using the supplied mat file
            %
            %   INPUT
            %       map         The path to the map file to load
            %   OUTPUT
            %       obj         A RobotMap Object
            %
            %   Cornell University
            %   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
            %   Alexius Wadell
            
            %Load map data
            switch class(map)
                case 'char'
                    %Load map data
                    map = load(map);
                    field = fieldnames(map);
                    
                    % Assign Wall Locations
                    wallField = findStructField(field, 'map');
                    obj.wall = map.(field{wallField});
                    
                    % Assign Beacon locations
                    beaconField = findStructField(field, 'beacon');
                    if ~isempty(beaconField)
                        obj.beacon = map.(field{beaconField});
                    end
                case 'double'
                    obj.wall = map;
            end
            
        end
        
        function plot(obj)
            % plot: Creates a plot of the map
            %
            %   INPUT
            %       RobotMap    A RobotMap Object
            %
            %   Cornell University
            %   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
            %   Alexius Wadell
            
            % Set up the axes for ploting
            box on; axis tight equal;
            
            % Check prior hold status
            priorHold = ishold;
            
            % Plot each of the walls
            hold on
            for i = 1:obj.nWall
                plot(obj.wall(i,[1,3]), obj.wall(i,[2,4]),...
                    'b-', 'LineWidth', 2);
            end
            hold off
            
            %Set Ticks -> Label every meter
            ax = gca;
            ax.XTick = ceil(ax.XTick(1)):1:floor(ax.XTick(end));
            ax.YTick = ceil(ax.YTick(1)):1:floor(ax.YTick(end));
            
            %Label Map Axes
            xlabel('X-Position [m]')
            ylabel('Y-Position [m]')
            
            %Restore Prior Hold
            if priorHold
                hold on
            else
                hold off
            end
        end
                
        function [x, y, w, h] = boundingBox(obj)
            % boundingBox: Returns the size and position of the bounding rectangle
            %
            %   INPUT
            %       RobotMap    A RobotMap Object
            %   OUTPUT
            %       x,y     position of lower left cornor of the box
            %       w,h     size of the box (w: along x, h: along y)
            %
            %   Cornell University
            %   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
            %   Alexius Wadell
            
            % Grab cordinates of wall endpoints
            xWall = obj.wall(:,[1, 3]); xWall = xWall(:);
            yWall = obj.wall(:,[2, 4]); yWall = yWall(:);
            
            %Calculate Position
            x = min(xWall);
            y = min(yWall);
            
            %Calculate Size
            w = range(xWall);
            h = range(yWall);
        end
        
        function [xl, yl, xu, yu] = mapCorners(obj)
            % mapConers: Returns the position of the upper left and lower
            % right corners of the bounding box
            %
            %   INPUT
            %       RobotMap    A RobotMap Object
            %   OUTPUT
            %       xl,yl     position of lower left corner of the bounding box
            %       xu, yu    position of upper right corner of the bounding box
            %
            %   Cornell University
            %   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
            %   Alexius Wadell
            
            % Grab bounding box
            [xl, yl, w, h] = obj.boundingBox;
            
            %Calculate Position of Upper Right corner
            xu = xl + w;
            yu = yl + h;
        end
        
        function inside = isInside(obj, point)
            %isInside: checks if the given point is within the map
            %
            %   INPUT
            %       obj     A Robot Map object
            %       point   The point to test [x, y] n-by-2 matrix
            %   OUTPUT
            %       inside  Logical for if the point is within the map.
            %               inside(i) -> gives status of point(i,:)
            %   NOTE: Assumes rectangular map
            
            [x, y, w, h] = obj.boundingBox;
            
            % Check if particle is within the map
            validx = point(:,1) >= x & point(:,1) <= x+w;
            validy = point(:,2) >= y & point(:,2) <= y+h;
            inside = validx & validy;
        end
        
        function mapGrid = getGrid(obj, m, n, inflate)
           % Returns a gridData object for representing the map
           
           % Get bounds of the map
           [xl, yl, xu, yu] = obj.mapCorners;
           
           if nargin < 4
               inflate = 0;
           end
           
           % Create gridData object -> Leave data as zeros
           mapGrid = gridData(linspace(xl-inflate, xu+inflate, m+1),...
                              linspace(yl-inflate, yu+inflate, n+1));
        end
        
        %% Define Access methods for Dependant Properties
        function n = get.nWall(obj)
            % nWall: Returns the number of walls in the current map
            
            n = size(obj.wall, 1);
        end
        
        %% Declare Methods defined externally
        [isect,x,y, dist] = intersectRay(obj, xo, yo, theta)    %Defined in intersectRay.m
        
        theta = intersectAngle(map, wallID, theta) %Defined in intersectAngle.m
    end
    
end
function I = findStructField(fieldname, pattern)

% Search for pattern in fieldnames
k = strfind(lower(fieldname), pattern);

% Find the first match
I = find(~cellfun(@isempty, k),1);
end
