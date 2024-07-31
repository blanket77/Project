from app import create_app

app = create_app() # Flask 앱 생성

if __name__ == '__main__': 
    app.run(host = "0.0.0.0", port=7000, debug=True) # Run the app