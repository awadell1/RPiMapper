function varargout = plot(obj, varargin)
% plot: Displays the gridData using a greyscale plot
%
%   INPUTS
%       obj         A gridData object
%       h           Pased via varargin. Handle to a previously generated gridData plot. Used to update a prior plot with new data.
%   OUTPUTS (Returned via varargout)
%       h       A handle to the Image graphics object created
%
%   Cornell University
%   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
%   Homework 5
%   Wadell, Alexius

switch nargin
    case 1
        % Plot Grided Data
        colormap (flipud(gray))
        h = imagesc(obj.cellX, obj.cellY, obj.data');
        xlabel('X-Position [m]')
        ylabel('Y-Position [m]')
        
        % Format Plot
        colorbar
        grid on
        box on
        
        % Keep Axis directions normal
        ax = gca;
        set(ax, 'XDir', 'normal');
        set(ax, 'YDir', 'normal');
    case 2
        % Update Plot
        h = varargin{1};
        set(h, 'XData', obj.cellX)
        set(h, 'YData', obj.cellY)
        set(h, 'CData', obj.data')
end

% Check if output was requested
if nargout == 1
    varargout{1} = h;
end
end