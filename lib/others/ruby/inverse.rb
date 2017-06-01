require 'csv'
require 'time'
filename_from="predict_diff_"+ARGV[0].to_s+".csv"
filename_to="predict_"+ARGV[0].to_s+".csv"
time="hoge"
ride_s="hoge"
ride_e="hoge"
open_time=Time.parse("08:05")
if ARGV[0].to_s == 120 || ARGV[0].to_s == 133 then
    ride_time=Time.parse("11:30")
else
    ride_time=Time.parse("08:40")
end
to_csv = CSV.generate do |csv|
  CSV.foreach(filename_from) do |origin|
    if origin[0].to_i == 0  then
        date=origin[2]
        time=open_time
        ride_s=ride_time
        ride_e=ride_s+3600
        new_row = [
          date,
          time.strftime("%H:%M"),
          ride_s.strftime("%H:%M"),
          ride_e.strftime("%H:%M")
        ]
    else
        date=origin[2]
        time=time+300
        ride_s=ride_s+origin[1].to_i*60
        ride_e=ride_s+3600
        if (ride_s-time <= 3600 || Time.parse("21:30") <= ride_s) && time >Time.parse("12:00")  then
            break;
        end
        new_row = [
          date,
          time.strftime("%H:%M"),
          ride_s.strftime("%H:%M"),
          ride_e.strftime("%H:%M")
        ]
    end
    csv << new_row
  end
end

File.open(filename_to, 'w') do |file|
  file.write(to_csv)
end
