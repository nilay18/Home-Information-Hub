from flask import (
Flask, render_template, redirect, session, url_for, request, jsonify)
from flask_login import (
LoginManager, login_user, logout_user, current_user, login_required)
from jinja2 import Template
import os
import requests
import google.oauth2.credentials
import google_auth_oauthlib.flow
import googleapiclient.discovery
import json
import base64
import email
from datetime import datetime
from dateutil.parser import parse as dtparse

os.environ['OAUTHLIB_INSECURE_TRANSPORT'] = '1'
os.environ['OAUTHLIB_RELAX_TOKEN_SCOPE'] = '1'
CLIENT_SECRETS_FILE = "client_secret.json"
SCOPES = ['https://www.googleapis.com/auth/userinfo.profile',
 'https://www.googleapis.com/auth/gmail.readonly',
 'https://www.googleapis.com/auth/calendar']
ONLY_LOGIN_SCOPES = ['https://www.googleapis.com/auth/userinfo.profile']
GMAIL_API_SERVICE_NAME = 'gmail'
CALENDAR_API_SERVICE_NAME = 'calendar'
GMAIL_API_VERSION = 'v1'
CALENDAR_API_VERSION = 'v3'
static_folder = 'src/'
app = Flask(__name__, static_folder=static_folder, static_url_path='')
app.secret_key = 'super secret key'
login_manager = LoginManager()
login_manager.init_app(app)
ACCOUNT_EMAIL = "coen315homeinformationhub@gmail.com"

PATH_TO_FILES_ON_EC2 = "/var/www/project/"
CREDENTIALS_FILE = "client_credentials.txt"
API_KEY_FILE = "api_key.txt"
PROXIMITY_FLAG_FILE = "proximity_flag.txt"
api_key_file_object = open(API_KEY_FILE,"r")
API_KEY = api_key_file_object.read()
WEATHER_API_KEY_FILE = "weather_api_key.txt"
weather_api_key_file_object = open(WEATHER_API_KEY_FILE, 'r')
WEATHER_API_KEY = weather_api_key_file_object.read().splitlines()[0]
LOCATIONS_FILE = "locations.txt"


class User():
    def __init__(self, id, first_name, last_name, email):
        self.id=id
        self.first_name=first_name
        self.last_name=last_name
        self.email=email
        self.is_active=True
        self.is_authenticated=True
        self.is_anonymous=False

    def get_id(self):
        return self.id


@login_manager.user_loader
def load_user(id):
    if id == 1:
        return User(1, "", "", ACCOUNT_EMAIL)
    else:
        return None


@app.route('/login')
def login(user_id):
    return "login page"


@app.route('/login_to_flask')
def login_to_flask():
  credentials = google.oauth2.credentials.Credentials(
      **session['credentials'])

  client = googleapiclient.discovery.build(
      GMAIL_API_SERVICE_NAME, GMAIL_API_VERSION, credentials=credentials)

  session['credentials'] = credentials_to_dict(credentials)

  data = client.users().getProfile(userId=ACCOUNT_EMAIL).execute()
  email = data['emailAddress']
  print(data)
  if email == ACCOUNT_EMAIL:
      user = User(1, "", "", ACCOUNT_EMAIL)
      login_user(user)
  return redirect(url_for('home'))


@app.route('/logout')
@login_required
def logout():
  logout_user()
  return redirect(url_for('clear_credentials'))

@app.route('/current_user')
@login_required
def current_user_page():
    print('test', current_user, 'test')
    return 'current user page'


@app.route('/gmail')
def gmail():
  with open(CREDENTIALS_FILE,"r") as fo:
    credentials_dict = json.loads(fo.read())

  credentials = google.oauth2.credentials.Credentials(
      **credentials_dict)

  service = googleapiclient.discovery.build(
      GMAIL_API_SERVICE_NAME, GMAIL_API_VERSION, credentials=credentials)

  with open(CREDENTIALS_FILE,"w") as fo:
    fo.write(json.dumps(credentials_to_dict(credentials)))

  emails_response = service.users().messages().list(userId=ACCOUNT_EMAIL).execute()['messages']

  for email_response in emails_response:
    message = service.users().messages().get(userId=ACCOUNT_EMAIL, id=email_response['id'],
    format='raw').execute()
    if 'UNREAD' in message['labelIds']:
      msg_str = base64.urlsafe_b64decode(message['raw'].encode('ASCII')).decode('utf-8')
      mime_msg = email.message_from_string(msg_str)
      date_received_dt = datetime.fromtimestamp(int(message['internalDate'])/1000)
      formatted_dt = (str(date_received_dt.month) + '/' +
        str(date_received_dt.day) + ' ' +
        str(date_received_dt.hour) + ':' +
        str(date_received_dt.minute))
      return jsonify({'title': mime_msg.__getitem__('Subject'),
                            'sender': mime_msg.__getitem__('From'),
                            'date_received': formatted_dt })
  return jsonify("")


@app.route('/maps')
def maps():
  locations_file = open(LOCATIONS_FILE,"r")
  locations = json.loads(locations_file.read())
  locations_file.close()
  start_location = locations['start_location_coordinates']
  end_location = locations['end_location_coordinates']
  r = requests.get(
  'https://maps.googleapis.com/maps/api/distancematrix/json?units=imperial' +
  '&origins=' + start_location +
  '&destinations=' + end_location +
  '&key=AIzaSyDOs1b0b8BVLre4PZQh4dSsqjw_mah-JCI')
  response = r.json()
  return jsonify(response)


@app.route('/calendar')
def calendar():
  with open(CREDENTIALS_FILE,"r") as fo:
    credentials_dict = json.loads(fo.read())

  credentials = google.oauth2.credentials.Credentials(
      **credentials_dict)

  service = googleapiclient.discovery.build(
      CALENDAR_API_SERVICE_NAME, CALENDAR_API_VERSION, credentials=credentials)

  with open(CREDENTIALS_FILE,"w") as fo:
    fo.write(json.dumps(credentials_to_dict(credentials)))


  events = service.events().list(calendarId='primary').execute()
  next_event_id = len(events['items']) - 1
  start_time_dt = events['items'][next_event_id]['start']['dateTime']
  tmfmt = '%m/%d %H:%M'

  stime = datetime.strftime(dtparse(start_time_dt), format=tmfmt)

  formatted_dt = stime
  calendar_dict = {'start_time': formatted_dt,
                   'title': events['items'][next_event_id]['summary']}
  return jsonify(calendar_dict)


@app.route('/current_weather')
def current_weather():
  print('http://api.openweathermap.org/data/2.5/weather?lat=37.35&lon=-121.96&appid='
   + WEATHER_API_KEY)
  r = requests.get(
  'http://api.openweathermap.org/data/2.5/weather?lat=37.35&lon=-121.96&appid='
   + WEATHER_API_KEY)
  response = r.json()
  formatted_response = {'name': response['name'],
                        'description': response['weather'][0]['description'],
                        'main': response['main']}
  return jsonify(formatted_response)


@app.route('/weather_forecast')
def weather_forecast():
  r = requests.get(
  'http://api.openweathermap.org/data/2.5/forecast?lat=37.35&lon=-121.96&appid='
   + WEATHER_API_KEY)
  response = r.json()
  formatted_response = {
  'name': response['city']['name'],
  'main': response['list'][0]['main'],
  'description': response['list'][0]['weather'][0]['description'],
  'datetime': response['list'][0]['dt_txt'],
  }
  return jsonify(formatted_response)


@app.route('/proximity')
def proximity():
  with open(PROXIMITY_FLAG_FILE,"r") as fo:
    flag_file = json.loads(fo.read())
    return jsonify(flag_file)


@app.route('/proximity_on')
def proximity_on():
  flag_file = open(PROXIMITY_FLAG_FILE,"r")
  with open(PROXIMITY_FLAG_FILE,"w") as fo:
    fo.write(json.dumps('ON'))
  return redirect(url_for('home'))

@app.route('/proximity_off')
def proximity_off():
  flag_file = open(PROXIMITY_FLAG_FILE,"r")
  with open(PROXIMITY_FLAG_FILE,"w") as fo:
    fo.write(json.dumps('OFF'))
  return redirect(url_for('home'))

@app.route('/authorize')
def authorize():
  fo = open(CREDENTIALS_FILE,"r")
  credentials_dict = json.loads(fo.read())

  if credentials_dict:
    flow = google_auth_oauthlib.flow.Flow.from_client_secrets_file(
        CLIENT_SECRETS_FILE, scopes=ONLY_LOGIN_SCOPES)
  else:
    flow = google_auth_oauthlib.flow.Flow.from_client_secrets_file(
        CLIENT_SECRETS_FILE, scopes=SCOPES)

  flow.redirect_uri = url_for('oauth2callback', _external=True)

  authorization_url, state = flow.authorization_url(
      access_type='offline',
      include_granted_scopes='true')

  session['state'] = state

  return redirect(authorization_url)


@app.route('/oauth2callback')
def oauth2callback():
  state = session['state']

  fo = open(CREDENTIALS_FILE,"r")
  credentials_dict = json.loads(fo.read())

  if credentials_dict:
    flow = google_auth_oauthlib.flow.Flow.from_client_secrets_file(
        CLIENT_SECRETS_FILE, scopes='ONLY_LOGIN_SCOPES', state=state)
  else:
    flow = google_auth_oauthlib.flow.Flow.from_client_secrets_file(
        CLIENT_SECRETS_FILE, scopes=SCOPES, state=state)
  flow.redirect_uri = url_for('oauth2callback', _external=True)
  authorization_response = request.url
  flow.fetch_token(authorization_response=authorization_response)
  credentials = flow.credentials
  session['credentials'] = credentials_to_dict(credentials)
  with open(CREDENTIALS_FILE,"w") as fo:
    fo.write(json.dumps(credentials_to_dict(credentials)))

  return redirect(url_for('login_to_flask'))


@app.route('/revoke')
def revoke():
  if 'credentials' not in session:
    return ('You need to <a href="/authorize">authorize</a> before ' +
            'testing the code to revoke credentials.')

  credentials = google.oauth2.credentials.Credentials(
    **session['credentials'])

  revoke = requests.post('https://accounts.google.com/o/oauth2/revoke',
      params={'token': credentials.token},
      headers = {'content-type': 'application/x-www-form-urlencoded'})

  status_code = getattr(revoke, 'status_code')
  if status_code == 200:
    return('Credentials successfully revoked.')
  else:
    return('An error occurred.')


@app.route('/clear')
def clear_credentials():
  if 'credentials' in session:
    del session['credentials']
  return ('Credentials have been cleared.<br><br>')


def credentials_to_dict(credentials):
  return {'token': credentials.token,
          'refresh_token': credentials.refresh_token,
          'token_uri': credentials.token_uri,
          'client_id': credentials.client_id,
          'client_secret': credentials.client_secret,
          'scopes': credentials.scopes}


@app.route('/')
def home():
  flag_file = open(PROXIMITY_FLAG_FILE,"r")
  flag = json.loads(flag_file.read())
  return render_template('index.html', proximity_flag=flag)


@app.route('/select_locations')
def select_locations():
  locations_file = open(LOCATIONS_FILE,"r")
  locations = json.loads(locations_file.read())
  start_location = locations['start_location']
  end_location = locations['end_location']
  return render_template('locations.html', start_location=start_location,
  end_location=end_location)


@app.route('/select_start_location')
def select_start_location():
  return render_template('address_geocode.html', destination='start_location')


@app.route('/select_end_location')
def select_end_location():
  return render_template('address_geocode.html', destination='end_location')


@app.route('/set_start_location', methods=["POST"])
def set_start_location():
  response = request.get_json(force=True)
  locations_file = open(LOCATIONS_FILE,"r")
  locations_dict = json.loads(locations_file.read())
  locations_file.close()

  address = response.get('address')
  coordinates = response.get('coordinates')
  coordinates = str(coordinates['lat']) + ', ' + str(coordinates['lng'])
  locations_dict['start_location'] = address
  locations_dict['start_location_coordinates'] = coordinates

  locations_file = open(LOCATIONS_FILE,"w")
  locations_file.write(json.dumps(locations_dict))
  locations_file.close()
  return jsonify(response)

@app.route('/set_end_location', methods=["POST"])
def set_end_location():
  response = request.get_json(force=True)
  locations_file = open(LOCATIONS_FILE,"r")
  locations_dict = json.loads(locations_file.read())
  locations_file.close()

  address = response.get('address')
  coordinates = response.get('coordinates')
  coordinates = str(coordinates['lat']) + ', ' + str(coordinates['lng'])
  locations_dict['end_location'] = address
  locations_dict['end_location_coordinates'] = coordinates

  locations_file = open(LOCATIONS_FILE,"w")
  locations_file.write(json.dumps(locations_dict))
  locations_file.close()
  return jsonify(response)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=80)
