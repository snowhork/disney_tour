require 'json'

class HomeController < ApplicationController
  def index
    @attractions = Attraction.all
    @areas       = Area.includes(:attractions)
  end

  def calc
    # pythonコール
    date_params =  params[:departed][:date].clone
    departed_time_params = params[:departed_time].clone
    if date_params.sub(%r{(\d{4})/(\d{2})/(\d{2})}, '\1\2\3') &&
        departed_time_params.sub(%r{(\d{2}):\d{2}}, '\1')
      system('python', "#{Rails.root.to_s}/lib/others/python/predict_wait_time.py", '20170401', '8', '1')
      #logger.info("execute python predict_wait_time.py #{date_params} #{departed_time_params} #{weather_state.to_s}")
    end

    # input_json を生成
    user_input_json_path = "#{Rails.root.to_s}/lib/others/cpp/input/user_input.json"
    user_input =
        {
            user: {
                list:  params[:attraction_ids].map { |k, v| { ID: k.to_i, hope: v.to_i} },
                start: params[:departed_time],
                end:   params[:finished_time],
                position: 0
            }
        }

    open(user_input_json_path, 'w') do |f|
      JSON.dump(user_input, f)
    end

    # Cコール
    logger.info('route_search.out')
    system("#{Rails.root.to_s}/lib/others/cpp/a.out", "#{Rails.root.to_s}/lib/others/cpp/")

    # 結果読み込み
    result = {}
    File.open("#{Rails.root.to_s}/lib/others/cpp/output/route_output.json") do |file|
      result = JSON.load(file)
    end
    logger.info(result)




    @candidates = result['candidates'].map do |candidate|
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
  class StartInfo
    attr_accessor :position_id, :attraction_name, :start_datetime
  end

  class AttractionInfo
    attr_accessor :algorithm_id, :attraction_name, :move_time, :arrive_time, :wait_time, :ride_time, :duration_time, :end_time
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
end