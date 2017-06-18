#coding:utf-8
require 'nokogiri'
require 'anemone'
require 'open-uri'
require 'csv'
PATTERN=/attrFastpass/
TEST=/年|月|火|水|木|金|土|日/
get=[]
start=[]
finish=[]
id=0
day="ほげ"
flag=0
site_id={"ビッグ" => 110,"スプラ" => 163 ,"モンス" => 123 ,"プーさ" => 112 ,"バズ・" => 134,"スター" => 133 ,"スペー" => 132 ,"ホーン" => 120}
Anemone.crawl("http://tokyodisneyresort.info/fastpass.php?park=land", depth_limit: 1) do |anemone|
    anemone.on_pages_like(PATTERN) do |page|
        if id != 4 then
            flag=0
            page.doc.xpath("//div[@id='contents']/div").each do |node|
                temp = node.xpath("./text()").to_s
                if temp.length == 11 && flag == 0 then
                    day = temp.delete!("年月日")
                    flag=1
                elsif temp.length == 11 && flag == 1 then
                    break
                elsif not temp =~ TEST then
                    get << temp.strip
                    time = node.xpath("./span/text()").to_s
                    start << time.slice(0,5)
                    finish << time.slice(6,5)
                end
            end
            title = page.doc.title.slice(0,3)
            filename="today_"+site_id[title].to_s+".csv"
            CSV.open(filename, "w") do |test|
                i=0
                get.each do |m|
                    test << [day,m,start[i],finish[i]]
                    i=i+1
                end
            end
            get=[]
            start=[]
            finish=[]
        end
        id=id+1
    end
end
