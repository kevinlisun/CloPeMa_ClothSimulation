#!/bin/bash

echo "Script for cleaning binaries from stack.";
echo "Author: Gerardo Aragon-Camarasa, Jan-2013"
echo

echo "Removing backup files."
find ./ -name '*~' | xargs rm
<<<<<<< HEAD
find ./ -name '*.pyc' | xargs rm
find ./ -name '*.pcd' | xargs rm
find ./ -name '*~' | xargs rm
find ./ -name 'test.mat' | xargs rm
find ./ -name 'capture_*.mat' | xargs rm

echo "Removing eclipse files."
find ./ -name '.cproject' | xargs rm
find ./ -name '.project' | xargs rm
find ./ -name '.pydevproject' | xargs rm
find ./ -name 'cmake_install.cmake' | xargs rm

echo "Removing qtcreator specific files."
find ./ -name 'CMakeLists.txt.user' | xargs rm

echo "Removing support files"
find ./ -name '*hDisp.txt' | xargs rm
find ./ -name '*vDisp.txt' | xargs rm
find ./ -name '*matchGPU*' | xargs rm
find ./ -name '*fMatch0*' | xargs rm
find ./ -name '*fMatch1*' | xargs rm
find ./ -name '*fMatch2*' | xargs rm
find ./ -name '*fRange0*' | xargs rn

find ./ -name 'fastMatcher_err.txt' | xargs rm
find ./ -name 'fastMatcher_out.txt' | xargs rm


echo "Removing calibration files"
#cd rh_calibration/calibrations/Debug/
find ./ -name '*_left_*.xml' | xargs rm
find ./ -name '*_right_*.xml' | xargs rm
find ./ -name '*_points_*.xml' | xargs rm
find ./ -name '*_pointsL*.xml' | xargs rm
find ./ -name '*_pointsR*.xml' | xargs rm

=======
>>>>>>> kevin-dev



