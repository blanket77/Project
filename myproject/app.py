from flask import Flask
from config import Config
from routes import main_bp
import os

def create_app(): # Flask 앱 생성 함수
    app = Flask(__name__) # Flask 앱 생성
    app.config.from_object(Config) # Config 클래스에서 설정을 가져옴
    app.config['UPLOAD_FOLDER'] = 'uploads/' # 업로드 폴더 설정

    # 업로드 폴더가 존재하지 않으면 생성
    if not os.path.exists(app.config['UPLOAD_FOLDER']): # 업로드 폴더가 존재하지 않으면
        os.makedirs(app.config['UPLOAD_FOLDER']) # 업로드 폴더 생성

    # 블루프린트 등록
    app.register_blueprint(main_bp) # main_bp 블루프린트 등록

    return app # Flask 앱 반환