from flask import request, jsonify, render_template, send_from_directory, request,  current_app
from services.one_stock_plotly import one_stock_plotly
from . import main_bp
from services.backtests.Static_Asset_Allocation import Static_Asset_Allocation
import os
from services.day_run import *
import json
from services.adjust_day_run import *

# 허용된 파일 확장자를 정의하는 함수
def allowed_file(filename):
    ALLOWED_EXTENSIONS = {'csv', 'pdf'}
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

# 메인 페이지 라우팅
@main_bp.route('/')
def home():
    return render_template('index.html')

# 백테스트 페이지 라우팅
@main_bp.route('/backtests')
def homes():
    return render_template('backtest.html')

# 그래프 페이지 라우팅
@main_bp.route('/showReport')
def showRepot():
    return send_from_directory('static', 'report.html')

# 업로드 페이지 라우팅
@main_bp.route('/up')
def up():
    return render_template('upload.html')

# 업로드 처리 라우팅
@main_bp.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return 'No file part', 400
    file = request.files['file']
    if file.filename == '':
        return 'No selected file', 400
    if file and allowed_file(file.filename):
        ext = os.path.splitext(file.filename)[1]  # 파일 확장자 추출
        fixed_filename = f"backtest.csv"  # 고정된 파일 이름 설정
        file.save(os.path.join(current_app.config['UPLOAD_FOLDER'], fixed_filename))
        return '', 204  # 성공 시 아무 내용 없이 응답
    return 'File type not allowed', 400

# 백테스트 처리 라우팅
@main_bp.route('/backtest', methods=['POST'])
def backtest():
    data = request.get_json()
    symbol = data.get('symbol')
    
    symbol = symbol.split(', ')
    symbol = [f'{item}' for item in symbol]

    start_date = data.get('startDate')
    end_date = data.get('endDate')

    # 백테스트 로직 호출
    results = Static_Asset_Allocation(symbol, start_date, end_date)

    return jsonify(results)

# 백테스트 종목별 수익률을 반환하는 라우팅
@main_bp.route('/day_backtest', methods=['POST'])
def day_backtest():
    run_portfolio_analysis()
    json_file_path = os.path.join(current_app.root_path, 'Record/stock_rate.json')

    # stock_rate.json 파일을 열고 JSON 데이터를 로드합니다.
    with open(json_file_path, 'r') as file:
        stock_data = json.load(file)

    # JSON 데이터를 클라이언트에 반환합니다.
    return jsonify(stock_data), 200

# 수정된 백테스트 종목별 수익률을 반환하는 라우팅
@main_bp.route('/day_backtest_adj', methods=['POST'])
def day_backtest_adj():
    json_file_path = os.path.join(current_app.root_path, 'Record/stock_rate_adj.json')

    # stock_rate.json 파일을 열고 JSON 데이터를 로드합니다.
    with open(json_file_path, 'r') as file:
        stock_data = json.load(file)

    # JSON 데이터를 클라이언트에 반환합니다.
    return jsonify(stock_data), 200


# 백테스트 종목별 빈도수를 반환하는 라우팅(매수, 매도, 빈도,ROI)
@main_bp.route('/sorted_stocks', methods=['POST'])
def sorted_stocks():
    json_file_path = os.path.join(current_app.root_path, 'Record/sorted_stock.json')

    # stock_rate.json 파일을 열고 JSON 데이터를 로드합니다.
    with open(json_file_path, 'r') as file:
        sorted_stock_data = json.load(file)

    # JSON 데이터를 클라이언트에 반환합니다.
    return jsonify(sorted_stock_data), 200

# 수정된 백테스트 종목별 빈도수를 반환하는 라우팅(매수, 매도, 빈도,ROI)
@main_bp.route('/sorted_stocks_adj', methods=['POST'])
def sorted_stocks_adj():
    json_file_path = os.path.join(current_app.root_path, 'Record/sorted_stock_adj.json')

    # stock_rate.json 파일을 열고 JSON 데이터를 로드합니다.
    with open(json_file_path, 'r') as file:
        sorted_stock_data = json.load(file)

    # JSON 데이터를 클라이언트에 반환합니다.
    return jsonify(sorted_stock_data), 200


# 백테스트 결과를 다운로드하는 라우팅
@main_bp.route('/download_report')
def download_report():
    return send_from_directory('static', 'report.html', as_attachment=True)

# 백테스트 결과를 보여주는 페이지 라우팅
@main_bp.route('/show_plot')
def show_plot():
    return render_template('backtest_report.html')

# 백테스트 결과를 보여주는 페이지 라우팅
@main_bp.route('/adj_day_backtest', methods=['POST'])
def adj_day_backtest():
    stock_name = request.form.get('stockName', 'ALB')  # 폼 데이터에서 stockName 값을 가져옴, 기본값은 'ALB'
    adj_run_portfolio_analysis(stock_name)
    return render_template('adj_day.html')

# 주식 데이터를 가져오는 라우팅
@main_bp.route('/get_stock_data', methods=['GET'])
def get_stock_data():
    stock_symbol = request.args.get('symbol')
    one_stock_plotly(stock_symbol)
    return send_from_directory('static', 'stock_plot.html')