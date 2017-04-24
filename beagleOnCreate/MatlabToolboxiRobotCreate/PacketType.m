classdef (Sealed) PacketType

properties  (Constant)
    INIT = 1;
    END = 2;
    CTRL = 3;
    DATA = 4;
	IMAGE = 5;
	SONAR = 6;
    ERROR = 7;
    SHUTDOWN = 8;
    UNKNOWN = 9;
end %constant properties
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
methods (Access = private)
%private so that you can't instatiate.
    function out = PacketType
    end %PacketType()
end %private methods
end %class PacketType