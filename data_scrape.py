import json
import requests
import sys
import datetime

class DataPoint:

  def __init__(self, response_data):
    self.air_quality = response_data['PM2_5']
    self.position = (response_data['Latitude'], response_data['Longitude'])
    self.time = self.ToLocalTime(response_data['time'])
    self.color = self.AqiToColor(self.air_quality)

  def ToLocalTime(self, time):

    months = {
      'Jan': 1,
      'Feb': 2,
      'Mar': 3,
      'Apr': 4,
      'May': 5,
      'Jun': 6,
      'Jul': 7,
      'Aug': 8,
      'Sep': 9,
      'Oct': 10,
      'Nov': 11,
      'Dec': 12
    }

    split_time = str.split(time, ' ')
    hms = str.split(split_time[4], ':')

    day = int(split_time[1])
    month = months[split_time[2]]
    year = int(split_time[3])
    hour = int(hms[0])
    minute = int(hms[1])
    second = int(hms[2])

    return datetime.datetime(year, month, day, hour, minute, second)

  def AqiToColor(self, air_quality):
    if (air_quality < 4):
      return '#31a354'
    elif (air_quality < 8):
      return '#a1d99b'
    elif (air_quality < 12):
      return '#e5f5e0'
    elif (air_quality < 20):
      return '#ffffcc'
    elif (air_quality < 28):
      return '#ffeda0'
    elif (air_quality < 35):
      return '#fed976'
    elif (air_quality < 42):
      return '#feb24c'
    elif (air_quality < 49):
      return '#fd8d3c'
    elif (air_quality < 55):
      return '#fc4e2a'
    elif (air_quality < 150):
      return '#e31a1c'
    elif (air_quality < 200):
      return '#bd0026'
    else:
      return '#800026'
    
  def __str__(self):
    s = 'Location: ' + str(self.position) + '\nAir Quality: ' + str(self.air_quality) + '\nTime: ' + str(self.time) + '\nColor: ' + self.color
    return s

url = 'https://www.aqandu.org/api/liveSensors?sensorSource=all'
response = requests.get(url)

if (not response.ok): 
  sys.exit('Web request failed with response: ' + response.status_code)

response_text = json.loads(response.text)
data_points = []

for entry in response_text:
  data_points.append(DataPoint(entry))

for item in data_points:
  print(item)
  print('\n')
