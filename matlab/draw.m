clc;clear;clf;clear serialportObj;

serialportObj = serialport("/dev/cu.usbmodem14203",115200);

configureCallback(serialportObj,"terminator",@readSerialData);

serialportObj.BaudRate = 115200;
serialportObj.DataBits = 7;
serialportObj.Parity = "odd";
serialportObj.UserData = struct("Step",[], "Angle",[], "Count",1);

figure(1);
xlim([-10, 10])
ylim([-10, 10])
% axis equal
% % view(3)
hold on
title("Magnetometer");

global x
global y
x = [0];
y = [0];

function readSerialData(serialportObj,~)
    step_length = 1.33;
    data = readline(serialportObj);
    
    data = split(data,",");
    D = str2double(data);
    
%     serialportObj.UserData.Step(end+1) = D(4);
%     serialportObj.UserData.Angle(end+1) = D(5);
    D
    global x
    global y
    x(serialportObj.UserData.Count + 1) = D(4);
    y(serialportObj.UserData.Count + 1) = D(5);

    serialportObj.UserData.Count = serialportObj.UserData.Count + 1;
    

%      plot3(serialportObj.UserData.X(2:end), serialportObj.UserData.Y(2:end), serialportObj.UserData.Z(2:end));

%     plot(serialportObj.UserData.X(2:end), 'r', 'LineWidth',2);
%     plot(serialportObj.UserData.Y(2:end), 'g', 'LineWidth',2);
%     plot(serialportObj.UserData.Z(2:end), 'b', 'LineWidth',2);
%     plot(serialportObj.UserData.A(2:end), 'r', 'LineWidth',2);
%     drawnow;
    x 
    y
    plot(x, y, "-");
    drawnow;
    if serialportObj.UserData.Count > 8
        
        configureCallback(serialportObj, "off");
        disp("Finished");
        clear serialportObj;
    end
end



