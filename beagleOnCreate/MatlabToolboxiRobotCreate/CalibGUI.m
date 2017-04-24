function varargout = CalibGUI(varargin)
% CALIBGUI M-file for CalibGUI.fig
%      CalibGUI, by itself, creates a new GUI or raises the existing
%      singleton*.
%
%      H = CalibGUI(ports) returns the handle to a new GUI or the handle to
%      the existing singleton*.
%
%      CalibGUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in CalibGUI.M with the given input arguments.
%
%      CalibGUI('Property','Value',...) creates a new GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before gui_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to gui_OpeningFcn via varargin.
%
%      *See CalibGUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: CreateBeagleInit, GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help CalibGUI

% Last Modified by GUIDE v2.5 26-Jan-2012 19:23:58

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @gui_OpeningFcn, ...
                   'gui_OutputFcn',  @gui_OutputFcn, ...
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


% --- Executes just before gui is made visible.
function gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to gui (see VARARGIN)

% Choose default command line output for gui
handles.output = hObject;
global DONE;
DONE = 0;
global ports;
if isempty(varargin)
    DONE = 1;
    fprintf(2, '??? Too few input argument, use CalibGUI(ports) to run this.\n');
else
    ports = varargin{1};
end

% send CTRL msg to beagleboard to enable video streaming
BeagleControl(ports.beagle, 3);

global TEST_DISTANCES;
TEST_DISTANCES = [2 3 4 5]*0.3048; % 2 to 5 foot, the unit is in meters
global d_measured;
d_measured = zeros(3,length(TEST_DISTANCES));
global sonarCalib;
sonarCalib = [0 0 0];
global SONAR_OFFSET;
SONAR_OFFSET = [0 0 0];

global cam_measured;
cam_measured = zeros(size(TEST_DISTANCES));
global cameraCalib;
cameraCalib = 0;
global BEACON_OFFSET;
BEACON_OFFSET = 0;
% Update handles structure
guidata(hObject, handles);
% UIWAIT makes gui wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = gui_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;
global ports;
global DONE;
global dist;
global tag;

while(DONE == 0)
    
%     PlotCreateVideo(1);

    tag = ReadBeacon(ports.beacon);
    if ~isempty(tag)
        set(handles.txtID,'String',num2str(tag(1,1)));
        set(handles.txtX,'String',num2str(tag(1,2)));
        set(handles.txtY,'String',num2str(tag(1,3)));
        set(handles.txtZ,'String',num2str(tag(1,4)));
        set(handles.txtYaw,'String',num2str(tag(1,5)));
    else
        set(handles.txtID,'String',num2str(0));
        set(handles.txtX,'String',num2str(0));
        set(handles.txtY,'String',num2str(0));
        set(handles.txtZ,'String',num2str(0));
        set(handles.txtYaw,'String',num2str(0));
    end
    dist = ReadSonar(ports.sonar, 0);
    dist = dist(1:3);
    set(handles.txtDist1,'String',num2str(dist(3)));
    set(handles.txtDist2,'String',num2str(dist(2)));
    set(handles.txtDist3,'String',num2str(dist(1)));

    pause(0.1)
end
% send CTRL msg to beagleboard to disable video streaming
BeagleControl(ports.beagle, 4);
close(gcf);


% --- Executes on button press in btnCalibCamera.
function btnCalibCamera_Callback(hObject, eventdata, handles)
% hObject    handle to btnCalibCamera (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global BEACON_OFFSET;
global tag;
global cameraCalib;
global TEST_DISTANCES;
global cam_measured;

j = mod(cameraCalib,length(TEST_DISTANCES)) + 1;
cameraCalib = cameraCalib + 1;
startstr = sprintf('');

if cameraCalib == 1
    BEACON_OFFSET = 0;
    % Show explanation message
    startstr = sprintf('\nStarting camera beacon calibration!\n(NOTE: For calibration, make sure you have good lighting');
end
str = strcat(startstr,sprintf('\n(Step %d/%d) Please place the ARtag %gcm (%gfeet) away', j, length(TEST_DISTANCES), TEST_DISTANCES(j)*100, TEST_DISTANCES(j)/0.3048));
set(handles.txtMsg,'String', str);

if cameraCalib == length(TEST_DISTANCES)
    set(handles.btnCalibCamera,'String', 'Done Calibration');
else
    set(handles.btnCalibCamera,'String', 'Next');
end

% Camera Calibration
if cameraCalib > 1
    j = mod(cameraCalib-2,length(TEST_DISTANCES)) + 1;
    if isempty(tag)
        cameraCalib = cameraCalib - 1;
        set(handles.txtMsg,'String', 'There is no ARtag detected in the view, click Next again when there is one.');
    else
        cam_measured(j) = tag(1,4);
    end

    if cameraCalib == length(TEST_DISTANCES)+1
        cameraCalib = 0;
        % Calculate offset
        BEACON_OFFSET = mean(TEST_DISTANCES - cam_measured);
        cam_measured = zeros(size(TEST_DISTANCES));
        % Save to file so you only have to calibrate once
        save beacon_calibration.mat BEACON_OFFSET;
        str = sprintf('\nCalibration complete!  BEACON_OFFSET is now %gm.\n', BEACON_OFFSET);
        set(handles.txtMsg,'String', str);
        set(handles.btnCalibCamera,'String', 'Calibrate Camera');
    end
end



% --- Executes on button press in btnCalibLeftSonar.
function btnCalibLeftSonar_Callback(hObject, eventdata, handles)
% hObject    handle to btnCalibLeftSonar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

CalibSonar(handles, 3);


% --- Executes on button press in btnCalibFrontSonar.
function btnCalibFrontSonar_Callback(hObject, eventdata, handles)
% hObject    handle to btnCalibFrontSonar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

CalibSonar(handles, 2);


% --- Executes on button press in btnCalibRightSonar.
function btnCalibRightSonar_Callback(hObject, eventdata, handles)
% hObject    handle to btnCalibRightSonar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

CalibSonar(handles, 1);

function CalibSonar(handles, sonarNum)
global SONAR_OFFSET;
global dist;
global sonarCalib;
global TEST_DISTANCES;
global d_measured;

sonarName{3} = 'Left';
sonarName{2} = 'Front';
sonarName{1} = 'Right';
btn = [handles.btnCalibRightSonar handles.btnCalibFrontSonar handles.btnCalibLeftSonar];

j = mod(sonarCalib(sonarNum),length(TEST_DISTANCES)) + 1;
sonarCalib(sonarNum) = sonarCalib(sonarNum) + 1;
startstr = sprintf('');
if sonarCalib(sonarNum) == 1
    SONAR_OFFSET(sonarNum) = 0;
    % Show explanation message
    startstr = sprintf('\nStarting sonar calibration!\n(NOTE: For calibration, it is recommended that you use a large, flat\nobject and hold it perpendicular to the direction the sonar sensor is facing.)');
end
str = strcat(startstr,sprintf('\nPlease place the object %gcm (%gfeet) away from sonar %s', TEST_DISTANCES(j)*100, TEST_DISTANCES(j)/0.3048, sonarName{sonarNum}));
set(handles.txtMsg,'String', str);

if sonarCalib(sonarNum) == length(TEST_DISTANCES)
    set(btn(sonarNum),'String', 'Done Calibration');
else
    set(btn(sonarNum),'String', 'Next');
end

% Sonar Calibration
if sonarCalib(sonarNum) > 1
    j = mod(sonarCalib(sonarNum)-2,length(TEST_DISTANCES)) + 1;

    if isnan(dist(sonarNum))
        sonarCalib(sonarNum) = sonarCalib(sonarNum) - 1;
        set(handles.txtMsg,'String', 'Sonar reading was NaN, click Next again');
    else
        d_measured(sonarNum,j) = dist(sonarNum);
    end

    if sonarCalib(sonarNum) == length(TEST_DISTANCES)+1
        sonarCalib(sonarNum) = 0;
        % Calculate offset
        SONAR_OFFSET(sonarNum) = mean(TEST_DISTANCES - d_measured(sonarNum,:));
        d_measured(sonarNum,:) = zeros(1,length(TEST_DISTANCES));
        % Save to file so you only have to calibrate once
        save sonar_calibration.mat SONAR_OFFSET;
        str = sprintf('\nCalibration complete! SONAR_OFFSET is now [%gm %gm %gm]', SONAR_OFFSET(1), SONAR_OFFSET(2), SONAR_OFFSET(3));
        set(handles.txtMsg,'String', str);
        str = sprintf('Calibrate %s Sonar', sonarName{sonarNum});
        set(btn(sonarNum),'String', str);
    end
end



% --- Executes on button press in btnDone.
function btnDone_Callback(hObject, eventdata, handles)
% hObject    handle to btnDone (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global DONE;
DONE = 1;
