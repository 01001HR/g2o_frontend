# Single scan from a planar laser range-finder
#
# If you have another ranging device with different behavior (e.g. a sonar
# array), please find or create a different message, since applications
# will make fairly laser-specific assumptions about this data

Header header            # timestamp in the header is the acquisition time of 
                         # the first ray in the scan.
                         #
                         # in frame frame_id, angles are measured around 
                         # the positive Z axis (clockwise, if Z is down) and
                         # around the positive Y axis (counterclockwise, if Y is on the left)
                         # with zero angle being forward along the x axis
                         
float32 azimuth_min        # start angle of the scan (orizontal) [rad]
float32 azimuth_max        # end angle of the scan (orizontal) [rad]
float32 azimuth_increment  # angular distance between measurements [rad]

float32 elevation_min        # start angle of the scan (vertical) [rad]
float32 elevation_max        # end angle of the scan (vertical) [rad]
float32 elevation_increment  # angular distance between measurements [rad]

float32 time_increment   # time between measurements [seconds] - if your scanner
                         # is moving, this will be used in interpolating position
                         # of 3d points
float32 scan_time        # time between scans [seconds]

float32 range_min        # minimum range value [m]
float32 range_max        # maximum range value [m]

geometry_msgs/Vector3[] measurements   # azimuth [rad], elevation [rad] and range data [m] (Note: values < range_min/azimuth_min/elevation_min or > range_max/azimuth_max/elevation_max should be discarded)
float32[] intensities                  # intensity data [device-specific units].  If your
                                       # device does not provide intensities, please leave
                                       # the array empty.
