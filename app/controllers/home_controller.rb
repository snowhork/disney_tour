require 'json'

class HomeController < ApplicationController
  def index
    @attractions = Attraction.all
    @areas       = Area.includes(:attractions)
  end

  def calc
    # pythonコール
    date_params =  params[:departed][:date].clone
    #date_params = '2017/04/29'
    departed_time_params = params[:departed_time].clone
    finished_time_params = params[:finished_time].clone

    unless date_params.sub!(%r{(\d{4})/(\d{2})/(\d{2})}, '\1\2\3') &&
        departed_time_params.match(%r{(\d{2}):(\d{2})}) &&
        finished_time_params.match(%r{(\d{2}):(\d{2})})
      return
    end

    user_input_json_path = "#{Rails.root.to_s}/lib/others/cpp/input/user_input.json"
    user_input =
        {
            user: {
                list:  params[:attraction_ids].map { |k, v| { ID: k.to_i, hope: v.to_i} },
                start: params[:departed_time],
                end:   params[:finished_time],
                position: 0,
                date: date_params
            }
        }

    open(user_input_json_path, 'w') do |f|
      JSON.dump(user_input, f)
    end

    # Cコール
    system("#{Rails.root.to_s}/lib/others/cpp/route_algorithm.out", "#{Rails.root.to_s}/lib/others/cpp/", date_params)

    depareted_minute = to_minute(departed_time_params)
    finished_minute = to_minute(finished_time_params)

    logger.info("date_params: #{date_params}, departed_minute: #{depareted_minute}, finished_minute: #{finished_minute}")
    system("#{Rails.root.to_s}/lib/others/cpp/kikaigakushu_once.out", "#{Rails.root.to_s}/lib/others/cpp/", date_params, depareted_minute, finished_minute)

    # 結果読み込み
    result = {}
    File.open("#{Rails.root.to_s}/lib/others/cpp/output/route_output.json") do |file|
      result = JSON.load(file)
    end

    result2 = {}
    File.open("#{Rails.root.to_s}/lib/others/cpp/output/route_output2.json") do |file|
      result2 = JSON.load(file)
    end


    @candidates = result['candidates'].map { |c| parse_candidate(c)}

    @candidates << parse_candidate(result2['candidates'][0])
  end

  private

  def weather_state
    weather_result = WeatherApi.new('Akashi-shi').execute
    logger.info(weather_result['list'].first['weather'].first)
    case weather_result['list'].first['weather'].first['main']
      when 'Clear'
        '1'
      when 'Clouds'
        '2'
      when 'Rain'
        '3'
      else
        '3'
    end
  end

  def attraction_name(attraction)
    name = Attraction.find_by(algorithm_id: attraction['ID']).name
    attraction['flag'] == 1 ? name + '(ファストパス)' : name
  end

  def to_minute(time)
    time.match(%r{(\d{2}):(\d{2})})
    ($1.to_i*60 + $2.to_i).to_s
  end

  def parse_candidate(candidate)
    { start: {
        id:     candidate['start']['place'],
        area_id: Attraction.find_by(algorithm_id: candidate['start']['place']).area_id,
        name: '出発' || Attraction.find_by(algorithm_id: candidate['start']['place']).name,
        time:  candidate['start']['time']
    },
      attractions: candidate['attraction'].map { |attraction|
        {
            id:    attraction['ID'],
            name:   attraction_name(attraction),
            area_id: Attraction.find_by(algorithm_id: attraction['ID']).area_id,
            start:      attraction['start'],
            move:       attraction['move'],
            arrive:     attraction['arrive'],
            wait:       attraction['wait'],
            ride:       attraction['ride'],
            duration:   attraction['duration'],
            end:        attraction['end']
        }
      },
      discription: candidate['discription']
    }
  end
end