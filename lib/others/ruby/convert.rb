require 'csv'
require 'time'
site_id={0 => 110,1 => 163 ,2 => 123 ,3 => 112,5 => 134,6 => 133 ,7 => 132 ,8 => 120}
day="ほげ"
previous="hoge"
for i in 0..8 do
    time=0
    if i != 4 then
        filename_from=site_id[i].to_s+"_fastpass.csv"
        filename_to="diff_"+site_id[i].to_s+".csv"
        to_csv = CSV.generate do |csv|
          CSV.foreach(filename_from) do |origin|
            if day != origin[0] then
                time=0
                day=origin[0]
                previous=Time.parse(origin[2])
                new_row = [
                  time,
                  "NaN",
                  day
                ]
            else
                now=Time.parse(origin[2])
                diff=(now-previous).to_i/60
                new_row = [
                  time,
                  diff.to_s,
                  day
                ]
                previous=now
            end
            time=time+1
            csv << new_row
          end
        end

        File.open(filename_to, 'w') do |file|
          file.write(to_csv)
        end
    end
end
