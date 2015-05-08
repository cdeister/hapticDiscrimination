% Parse Behavior

%% flatten epochs

% The data is acquired in chunks for sanity/saftey sake. Each epoch is an
% entry in the cell arrays within "data."


% first make a flat time vector.
flatData.time(:,1)=data.totalTime{1};
for n=2:numel(data.totalTime)
    dataToAdd=data.totalTime{n}+flatData.time(end);
    flatData.time=vertcat(flatData.time,dataToAdd');
end
clear n dataToAdd

% next we 