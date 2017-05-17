function savePlot(name)
%Save Plot as epsc and jpeg

% Check for the image folder
if ~exist('img', 'dir')
    mkdir('img');
end

%Save as EPS and JPEG
saveas(gcf, fullfile('img', name), 'epsc');
saveas(gcf, fullfile('img', name), 'jpg');
end