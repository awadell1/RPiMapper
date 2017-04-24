function [image size] = PlotCreateVideo(showplot)
%PLOTCREATEVIDEO  Plot the video feed of the camera on the Create.
%   PlotCreateVideo(showplot) returns
%   image = image matrix of one image frame
%   size = the size of the image
%
%   INPUT: showplot ~= 0 turns on ploting image
%
%   NOTE: To plot the image, first check the size > 0, then
%   use imshow(image,'DisplayRange',[0 255])
%
% By: Chuck Yang, ty244, 2012

try
    %remote port (integer value)
    port = 8855;
    imagesize = 19200;
    mssg = judp('receive',port,imagesize+20);
    packet = mssg;

    size = length(packet);
    if (size > 13)
        width = typecast(int8(packet(13:16)),'uint32');
        height = typecast(int8(packet(17:20)),'uint32');
        packet = typecast(int8(packet(21:end)), 'uint8');
        image = reshape(packet, width, height)';

        size = width*height;
    else
        image = 0;
        size = 0;
    end
    if size ~= 0 && showplot ~= 0
        imshow(image,'DisplayRange',[0 255])
    end
catch 
%     disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
end
end