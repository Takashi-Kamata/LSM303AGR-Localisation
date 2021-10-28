clc;clear;clf;clear serialportObj;

serialportObj = serialport("/dev/cu.usbmodem1413203",115200);

configureCallback(serialportObj,"terminator",@readSerialData);

serialportObj.BaudRate = 115200;
serialportObj.DataBits = 7;
serialportObj.Parity = "odd";
serialportObj.UserData = struct("X",[], "Y",[], "Z",[], "A", [] ,"Count",1);

figure(1);
xlim([0 200])
ylim([-0 360])
% axis equal
% view(3)
hold on
title("Magnetometer");

function readSerialData(serialportObj,~)
    data = readline(serialportObj);
    
    data = split(data,",");
    X = str2double(data);
    
    serialportObj.UserData.X(end+1) = X(1);
    serialportObj.UserData.Y(end+1) = X(2);
    serialportObj.UserData.Z(end+1) = X(3);
    serialportObj.UserData.A(end+1) = X(4);
    X(4);

    serialportObj.UserData.Count = serialportObj.UserData.Count + 1;

%     plot3(serialportObj.UserData.X(2:end), serialportObj.UserData.Y(2:end), serialportObj.UserData.Z(2:end));


%     plot(serialportObj.UserData.X(2:end), 'r', 'LineWidth',2);
%     plot(serialportObj.UserData.Y(2:end), 'g', 'LineWidth',2);
%     plot(serialportObj.UserData.Z(2:end), 'b', 'LineWidth',2);
     plot(serialportObj.UserData.A(2:end), 'r', 'LineWidth',2);
    drawnow;
    
    if serialportObj.UserData.Count > 200
        configureCallback(serialportObj, "off");
        legend("X", "Y", "Z");
        disp("Finished");
        clear serialportObj;
    end
end



