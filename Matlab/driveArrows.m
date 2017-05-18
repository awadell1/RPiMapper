function varargout = driveArrows(varargin)
% driveArrows initializes a GUI (driveArrows.fig) that displays the
% linear and angular velocity of the Create robot.  The velocity can be
% controlled using the arrow keys or WASD.
% Up arrow or W - increase linear velocity
% Down arrow or S - decrease linear velocity
% Left arrow or A - increase angular velocity
% Right arrow or D - decrease angular velocity
% Space, Enter, or Esc - zero both velocities
% 
% Input argument must be the serial port object for controlling the Create
% (the output argument from RoombaInit)
% 
% There are no output arguments

% Edit the above text to modify the response to help driveArrows

% Last Modified by GUIDE v2.5 15-Mar-2011 12:10:30

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @driveArrows_OpeningFcn, ...
                   'gui_OutputFcn',  @driveArrows_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before driveArrows is made visible.
function driveArrows_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to driveArrows (see VARARGIN)

% Choose default command line output for driveArrows
handles.output = hObject;

% Set robot object to figure user data
set(handles.figure_create,'UserData',varargin{1})
obj= get(handles.figure_create,'UserData');

% Update handles structure
guidata(hObject, handles);


% UIWAIT makes driveArrows wait for user response (see UIRESUME)
% uiwait(handles.figure_create);


% --- Outputs from this function are returned to the command line.
function varargout = driveArrows_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on key press with focus on figure_create or any of its controls.
function figure_create_WindowKeyPressFcn(hObject, eventdata, handles)
% hObject    handle to figure_create (see GCBO)
% eventdata  structure with the following fields (see FIGURE)
%	Key: name of the key that was pressed, in lower case
%	Character: character interpretation of the key(s) that was pressed
%	Modifier: name(s) of the modifier key(s) (i.e., control, shift) pressed
% handles    structure with handles and user data (see GUIDATA)

% Get current velocities
fwdvel= str2double(get(handles.text_fwdvel,'String'));
angvel= str2double(get(handles.text_angvel,'String'));
obj= get(handles.figure_create,'UserData');

% Check which key is pressed and set data appropriately
keydown= eventdata.Key;  % Assume only one key pressed at a time
if any(strcmp(keydown,{'uparrow' 'w'}))
    fwdvel= fwdvel+0.01;
elseif any(strcmp(keydown,{'downarrow' 's'}))
    fwdvel= fwdvel-0.01;
elseif any(strcmp(keydown,{'leftarrow' 'a'}))
    angvel= angvel+0.01;
elseif any(strcmp(keydown,{'rightarrow' 'd'}))
    angvel= angvel-0.01;
elseif any(strcmp(keydown,{'space' 'return' 'escape'}))
    fwdvel= 0;
    angvel= 0;
end

% Set velocities and update display
obj.SetFwdVelAngVel(fwdvel, angvel);


set(handles.text_fwdvel,'String',sprintf('%1.2f',fwdvel))
set(handles.text_angvel,'String',sprintf('%1.2f',angvel))


guidata(hObject, handles);
