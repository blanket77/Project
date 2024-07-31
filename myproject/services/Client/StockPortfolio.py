import plotly.graph_objects as go
from datetime import datetime, timedelta
import copy
from collections import OrderedDict
from sqlalchemy import create_engine
import pandas as pd
import json
import os
from collections import Counter
from functools import reduce
import plotly.io as pio

# 매수 수수료
buy_commission = 0 
# 매도 수수료
sell_commission = 0 

# 주식 포트폴리오 클래스
class StockPortfolio:
    def __init__(self, date, cash):
        self.date = datetime.strptime(date, '%Y-%m-%d')
        self.cash = cash
        self.cash_plus_stock = cash
        self.initial_cash = cash
        self.stocks = {}
        self.daily_history = {}
        self.memory_date = datetime.strptime(date, '%Y-%m-%d')
        engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')
        self.stock_price = pd.read_sql('select * from lab_assignment;', con=engine)
        engine.dispose()
        self.fig = go.Figure()

    # 주식 종목 추가
    def initialization_stock(self):

        if(self.memory_date != self.date):
            for ticker in self.stocks:
                self.stocks[ticker]['quantity_buy'] = 0
                self.stocks[ticker]['quantity_sold'] = 0
                self.stocks[ticker]['return_stock_selling'] = 0
            self.memory_date = self.date

    # 주식 종목 추가
    def add_stock(self, ticker, price, quantity):

        if ticker not in self.stocks:
            self.stocks[ticker] = {
                'price': price,
                'quantity': quantity,
                'total_bought': quantity * price * (1 + buy_commission),
                'total_sold': 0,
                'quantity_buy':quantity,
                'quantity_sold': 0,
                'recovery_bought': quantity * price * (1 + buy_commission),
                'return_stock_selling': 0,
            }
        else:
            print(f"Stock {ticker} already exists in the portfolio.")

    # 주식 매수
    def buy_stock(self, date, ticker, price, quantity):
        date = datetime.strptime(date, '%Y-%m-%d')
        self.date = date

        self.initialization_stock()

        if ticker in self.stocks:
            self.stocks[ticker]['price'] = price
            self.stocks[ticker]['quantity'] += quantity
            self.stocks[ticker]['total_bought'] += quantity * price * (1 + buy_commission)
            self.stocks[ticker]['recovery_bought'] += quantity * price * (1 + buy_commission)
            self.stocks[ticker]['quantity_buy'] = quantity
            # self.stocks[ticker]['quantity_sold'] = 0
            self.cash -= price * quantity * (1 + buy_commission)
            # self.stocks[ticker]['return_stock_selling'] = 0

        else:
            self.add_stock(ticker, price, quantity)
            self.cash -= price * quantity * (1 + buy_commission)

        self.cash_plus_stock = self.get_cash_plus_stocks()
        self.update_daily_history(date)

    # 주식 판매
    def sell_stock(self, date, ticker, price, quantity):
        date = datetime.strptime(date, '%Y-%m-%d')
        self.date = date

        # 초기화
        self.initialization_stock()

        if ticker in self.stocks:
            if self.stocks[ticker]['quantity'] >= quantity:
                self.stocks[ticker]['price'] = price

                # 누적수익률 구하려고 함
                buy = (self.stocks[ticker]['recovery_bought'] / self.stocks[ticker]['quantity'] * quantity)
                sell = price * quantity
                self.stocks[ticker]['return_stock_selling'] = (sell - buy) / buy

                self.stocks[ticker]['recovery_bought'] -= self.stocks[ticker]['recovery_bought'] / self.stocks[ticker]['quantity'] * quantity

                self.stocks[ticker]['quantity'] -= quantity
                self.stocks[ticker]['total_sold'] += price * quantity * (1 - sell_commission)
                # self.stocks[ticker]['quantity_buy'] = 0
                self.stocks[ticker]['quantity_sold'] = quantity

                self.cash += price * quantity * (1 - sell_commission)
            else:
                print(f"Not enough quantity of {ticker} to sell.")
        else:
            print(f"Stock {ticker} not found in the portfolio.")

        self.cash_plus_stock = self.get_cash_plus_stocks()
        self.update_daily_history(date)

    # 현금 + 주식 가치 계산
    def get_cash_plus_stocks(self):
        cash_plus_stock = 0
        cash_plus_stock += self.cash
        for ticker, data in self.stocks.items():
            cash_plus_stock += data['price'] * data['quantity'] * (1 - sell_commission)
        return cash_plus_stock

    # 모든 주식 종목을 판매
    def sell_all_stocks(self):
        # 가장 최근 날짜를 가져옴
        if self.daily_history:
            last_date = max(self.daily_history.keys())
        else:
            last_date = self.date.strftime('%Y-%m-%d')

        total_value = 0
        # 모든 주식 종목을 판매
        for ticker, data in self.stocks.items():
            total_value += data['price'] * data['quantity'] # 총 가치 계산
            self.cash += data['price'] * data['quantity'] * (1 - sell_commission) # 현금 업데이트 
            data['total_sold'] += data['price'] * data['quantity'] # 총 매도량 업데이트
            data['quantity_sold'] += data['quantity'] # 매도량 업데이트
            data['quantity'] = 0 # 주식 보유량을 0으로 설정

        # 가장 최근 날짜로 기록 업데이트
        self.update_daily_history(datetime.strptime(last_date, '%Y-%m-%d'))
        return total_value

    # 포트폴리오의 수익률 계산
    def calculate_return_rate(self):
        # 포트폴리오의 복사본을 생성
        portfolio_copy = copy.deepcopy(self) # 포트폴리오의 복사본을 생성
        current_value = self.cash + portfolio_copy.sell_all_stocks() # 현재 가치 계산
        return_rate = ((current_value - self.initial_cash) / self.initial_cash) * 100 # 수익률 계산
        return return_rate # 수익률 반환

    # 주식 종목의 수익률 계산
    def calculate_stock_return_rate(self, ticker):
        if ticker in self.stocks:
            data = self.stocks[ticker] # 주식 종목 데이터 가져오기
            if data['total_bought'] > 0: # 매수 금액이 0보다 큰 경우
                stock_return_rate = ((data['total_sold'] - data['total_bought']) / data['total_bought']) # ROI 계산
                return stock_return_rate # 수익률 반환
            else:
                return 0
        else:
            print(f"Stock {ticker} not found in the portfolio.") # 주식 종목이 포트폴리오에 없는 경우
            return None # None 반환

    # 모든 주식 종목의 수익률 계산
    def calculate_all_stock_return_rates(self, file_name): # 주식 종목의 수익률 계산
        stock_return_rates = {} # 주식 종목별 수익률을 저장할 딕셔너리
        portfolio_copy = copy.deepcopy(self) # 포트폴리오의 복사본을 생성

        total_value = 0 # 총 가치
        for ticker, data in portfolio_copy.stocks.items(): # 주식 종목에 대해 반복
            total_value += data['price'] * data['quantity'] # 총 가치 계산
            portfolio_copy.cash += data['price'] * data['quantity'] * (1 - sell_commission) # 현금 업데이트
            data['total_sold'] += data['price'] * data['quantity'] # 총 매도량 업데이트 
            data['quantity_sold'] += data['quantity']   # 매도량 업데이트
            data['quantity'] = 0 # 주식 보유량을 0으로 설정

        for ticker, data in portfolio_copy.stocks.items(): # 주식 종목에 대해 반복
            stock_return_rate = portfolio_copy.calculate_stock_return_rate(ticker) # 주식 종목의 수익률 계산
            stock_return_rates[ticker] = {
                'return_rate': stock_return_rate * 100, # 수익률을 백분율로 변환
            }

        directory = './Record'
        if not os.path.exists(directory): # 디렉토리가 없는 경우
            os.makedirs(directory) # 디렉토리 생성

        file_path = os.path.join(directory, file_name) # 파일 경로 설정

        with open(file_path, 'w') as file: # 파일 열기
            json.dump(stock_return_rates, file, indent=4) # JSON 파일로 저장

        return stock_return_rates # 주식 종목별 수익률 반환

    # 일별 기록을 업데이트
    def update_daily_history(self, date):
        # 주식 종목의 복사본을 생성
        tmp =  copy.deepcopy(self.stocks)

        # 기록을 저장
        self.daily_history[date.strftime('%Y-%m-%d')] = { # 날짜를 문자열로 변환하여 키로 사용
            'cash': self.cash, # 현금
            'cash_plus_stock': self.cash_plus_stock, # 현금 및 주식 가치
            'stocks': tmp # 주식 종목
        }
        # self.cash_plus_stock = self.get_cash_plus_stocks()

    # 포트폴리오 요약을 가져옴
    def get_portfolio_summary(self): 
        summary = { # 요약 정보를 딕셔너리로 저장
            'date': self.date, # 날짜
            'cash': self.cash, # 현금
            'cash_plus_stock': self.cash_plus_stock, # 현금 및 주식 가치
            'stocks': self.stocks, # 주식 종목
            # 'daily_history': self.daily_history
        }
        return summary # 요약 정보 반환

    # 일별 기록을 가져옴
    def get_daily_history(self):
        return self.daily_history  
    
    # 일별 기록을 파일로 저장
    def get_daily_history_file(self, file_name):    
        directory = './Record' # 디렉토리 경로 설정
        if not os.path.exists(directory): # 디렉토리가 없는 경우
            os.makedirs(directory) # 디렉토리 생성

        file_path = os.path.join(directory, file_name) # 파일 경로 설정

        with open(file_path, 'w') as file: # 파일 열기
            json.dump(self.daily_history, file, indent=4) # JSON 파일로 저장

    # 누락된 날짜를 채워줌
    def fill_missing_dates(self):
        if not self.daily_history: # daily_history가 비어있는 경우
            return
        
        start_date = min(self.daily_history.keys()) # 가장 오래된 날짜를 가져옴
        end_date = max(self.daily_history.keys()) # 가장 최근 날짜를 가져옴
        
        # stock_price = pd.read_sql('select * from lab_assignment2_open;', con=engine)
        self.stock_price['Date'] = pd.to_datetime(self.stock_price['Date']).dt.date.astype(str) # 날짜를 문자열로 변환
        self.stock_price = self.stock_price.set_index(['Date']) # 이걸로 인덱스 설정하겠다.
        
        # print(stock_price.index)
        # print(type(stock_price.index))
        
        current_date = datetime.strptime(start_date, '%Y-%m-%d') # 시작 날짜를 datetime 형식으로 변환
        end_date = datetime.strptime(end_date, '%Y-%m-%d') # 끝 날짜를 datetime 형식으로 변환
        
        # 시작 날짜부터 끝 날짜까지 하루씩 증가시키면서 반복합니다.
        while current_date <= end_date:
            date_str = current_date.strftime('%Y-%m-%d') # 날짜를 문자열로 변환
            if date_str in self.stock_price.index: # 날짜가 주식 가격 데이터에 있는 경우
                if date_str not in self.daily_history: # 날짜가 daily_history에 없으면
                    previous_date = (current_date - timedelta(days=1)).strftime('%Y-%m-%d')
                    # 이전 날짜가 daily_history에 있으면, 그 값을 깊은 복사하여 현재 날짜에 추가합니다.
                    if previous_date in self.daily_history: # 이전 날짜가 daily_history에 있는 경우
                        temp = copy.deepcopy(self.daily_history[previous_date]) # 이전 날짜의 데이터를 깊은 복사
                        
                    for ticker in temp['stocks']: # 주식 종목에 대해 반복
                        temp['stocks'][ticker]['price'] = self.stock_price.loc[date_str, ticker] # 주식 가격을 업데이트
                        temp['stocks'][ticker]['quantity_buy'] = 0 # 매수량을 0으로 설정
                        temp['stocks'][ticker]['quantity_sold'] = 0 # 매도량을 0으로 설정
                        temp['stocks'][ticker]['return_stock_selling'] = 0 # 수익률을 0으로 설정 
                        temp['cash_plus_stock'] = self.get_cash_plus_stocks() # 현금 및 주식 가치를 업데이트
                    self.daily_history[date_str] = temp # 현재 날짜의 데이터를 업데이트
            current_date += timedelta(days=1) # 날짜를 하루 증가시킴

        # 날짜 순으로 정렬
        self.daily_history = OrderedDict(sorted(self.daily_history.items())) 
    
    # 이전 그래프를 불러오기
    def previous_fig_json(self, json_path):
        self.fig = pio.read_json(json_path)  # 이전 그래프를 불러오기

    # 수익률 그래프 그리기
    def plot_rate_of_return_history(self): 
        self.fill_missing_dates()  # Fill in missing dates before plotting

        spy_price = [] 
        dates = []
        cash_plus_stock_values = []
        buy_annotations = {}
        sell_annotations = {}
        returns = []

        # 날짜, SPY 가격, 포트폴리오 가치, 수익률 계산
        for date_str, data in self.daily_history.items():
            date = datetime.strptime(date_str, '%Y-%m-%d') # 날짜를 datetime 형식으로 변환
            dates.append(date) # 날짜 추가
            spy_price.append(self.stock_price['SPY'][date_str]) # SPY 가격 추가

            cash_plus_stock_values.append(data['cash_plus_stock']) # 포트폴리오 가치 추가

            current_value = data['cash_plus_stock'] # 현재 자산 가치
            returns.append(round((current_value - self.initial_cash) / self.initial_cash * 100, 2)) # 수익률 계산

            # 매수, 매도 주석 추가
            for ticker, stock_data in data['stocks'].items(): 
                if stock_data['quantity_buy'] > 0:
                    # 매수 주석 추가
                    if date not in buy_annotations:
                        buy_annotations[date] = [] # 날짜가 buy_annotations에 없으면 빈 리스트를 추가
                    buy_annotations[date].append(f'{ticker}: {stock_data["quantity_buy"] * stock_data["price"] / data["cash_plus_stock"] * 100 :.1f}%') # 주식 종목의 비율 추가
                if stock_data['quantity_sold'] > 0:
                    # 매도 주석 추가
                    if date not in sell_annotations:
                        sell_annotations[date] = []
                    sell_annotations[date].append(
                    f'{ticker}: {stock_data["quantity_sold"] * stock_data["price"] / data["cash_plus_stock"] * 100 :.1f}%, '  # 주식 종목의 비율 추가
                    f'ROI: {stock_data["return_stock_selling"]:.1%}' # 수익률 추가
                )

            # 누적 수익률 계산
            if date in buy_annotations or date in sell_annotations:
                # 매수, 매도 주석이 있는 경우, 현금 비율 추가
                if date in buy_annotations:
                    buy_annotations[date].append(f'Cash: {data["cash"] / data["cash_plus_stock"] * 100:.1f}%') # 현금 비율 추가
                    continue
                if date in sell_annotations:
                    sell_annotations[date].append(f'Cash: {data["cash"] / data["cash_plus_stock"] * 100:.1f}%') # 현금 비율 추가

    
        # 누적 수익률 계산
        spy_price = (spy_price - spy_price[0]) / spy_price[0] * 100
        spy_price = [round(price, 2) for price in spy_price]

        #SPY 수익률과 포트폴리오 수익률을 그래프와 라벨로 표시
        spy_trace_exists = any(trace.name == 'SPY' for trace in self.fig.data)
        
        # 기존 그래프에 있는 트레이스 이름 가져오기
        existing_names = {trace.name for trace in self.fig.data}
        i = 1
        while f'My{i}' in existing_names: # My1, My2, My3, ... 순서로 트레이스 이름이 있는지 확인
            i += 1 

        # SPY 수익률 그래프 추가
        if not spy_trace_exists:
            self.fig.add_trace(go.Scatter(
                x=dates, y=spy_price, mode='lines+markers', name='SPY', # 트레이스 이름 설정
                hovertemplate=    # Hover 텍스트 템플릿 설정
                        'Date: %{x|%d/%m/%Y}<br>' + # 날짜 추가
                        'CR: %{y:.1f}%<br>', # 누적 수익률 추가

                line=dict(color='purple'),  # 선 색깔을 보라색으로 설정
                hovertext=[f"{price}%" for price in spy_price],  # Hover 텍스트에 % 기호 추가
                hoverlabel=dict(font=dict(size=16)),  # Hover 텍스트 폰트 크기 설정
                showlegend=True # 범례 표시
            ))

        # 포트폴리오 수익률 그래프 추가
        self.fig.add_trace(go.Scatter(
            x=dates, y=returns, mode='lines+markers', name=f'My{i}', # 트레이스 이름 설정
            hovertemplate=    # Hover 텍스트 템플릿 설정
                        'Date: %{x|%d/%m/%Y}<br>' +  # 날짜 추가
                        'CR: %{y:.1f}%<br>',  # 누적 수익률 추가
                              
            hovertext=[f"{price}%" for price in returns],  # Hover 텍스트에 % 기호 추가
            hoverlabel=dict(font=dict(size=16)),  # Hover 텍스트 폰트 크기 설정
            showlegend=True, # 범례 표시
            legendgroup=f'my_group{i}' # 범례 그룹 설정
        ))
        
        # 매수 주석 추가
        for date, annotations in buy_annotations.items():
            self.fig.add_trace(go.Scatter(
                x=[date + timedelta(minutes=400)], y=[returns[dates.index(date)]], # 주석을 추가할 x, y 좌표 설정
                mode='markers', name='Buy', # 모드를 마커로 설정
                marker=dict(color='red', symbol='triangle-up', size=10), # 마커 색깔을 빨간색으로, 모양을 삼각형으로, 크기를 10으로 설정
                text='<br>'.join(annotations), # 주석 텍스트 설정
                hoverlabel=dict(font=dict(size=20)), # Hover 텍스트 폰트 크기 설정
                hovertemplate=    # Hover 텍스트 템플릿 설정
                        'Date: %{x|%d/%m/%Y}<br>' +
                        'CR: %{y:.1f}%<br><br>' +
                        '<br>'.join(sorted(annotations, key=lambda x: float(x.split(': ')[1].rstrip('%')), reverse=True)),
                              
                showlegend=False, # 범례 표시 안 함
                legendgroup=f'my_group{i}' # 범례 그룹 설정
            ))

        # 매도 주석 추가
        for date, annotations in sell_annotations.items(): # 매도 주석 추가
            self.fig.add_trace(go.Scatter( 
                x=[date], y=[returns[dates.index(date)]], # 주석을 추가할 x, y 좌표 설정
                mode='markers', name='Sell', # 모드를 마커로 설정
                marker=dict(color='blue', symbol='triangle-down', size=10), # 마커 색깔을 파란색으로, 모양을 역삼각형으로, 크기를 10으로 설정
                text='<br>'.join(annotations), # 주석 텍스트 설정
                hoverlabel=dict(font=dict(size=20)), # Hover 텍스트 폰트 크기 설정
                                hovertemplate=     # Hover 텍스트 템플릿 설정
                        'Date: %{x|%d/%m/%Y}<br>' +
                        'CR: %{y:.1f}%<br><br>' +
                        '<br>'.join(sorted(annotations, key=lambda x: float(x.split('%')[0].split(': ')[-1].rstrip('%')), reverse=True)), # Hover 텍스트 템플릿 설정
                              
                showlegend=False, # 범례 표시 안 함
                legendgroup=f'my_group{i}' # 범례 그룹 설정
            ))

        # 그래프 레이아웃 설정
        self.fig.update_layout(
                        hoverlabel_align = 'right', # Hover 텍스트 오른쪽 정렬
                        title='Cumulative Return(CR)', # 제목 설정
                        xaxis_title='Date', # x축 제목 설정
                        yaxis_title='Cumulative Return(%)', # y축 제목 설정
                        showlegend=True, # 범례 표시
                        font=dict(size=25),  # 글꼴 크기 변경
                        title_font=dict(size=40),  # 제목 글꼴 크기 변경
                        xaxis=dict(title=dict(font=dict(size=30))), # x축 글꼴 크기 변경
                        yaxis=dict(title=dict(font=dict(size=30))), # y축 글꼴 크기 변경
                        legend=dict(font=dict(size=40))  # 범례 글꼴 크기 변경            
                    )
        pio.write_json(self.fig, 'Record/day_graph.json') # 그래프를 json 파일로 저장, 이전 그래프 계속 누적되게 한다.

    # 주식 종목별 통계 계산(매수, 매도, ROI < 0%, ROI < -4%, ROI < -8%, ROI < -12%)
    def statistics_stock(self, file_path, json_path):

        # JSON 파일 읽기
        with open(file_path, 'r') as file:
            data = json.load(file)

        # 'quantity_buy'가 0이 아닌 경우의 종목별 빈도 계산
        quantity_buy_nonzero = []
        quantity_sold_nonzero = []
        Retun_less_0per = []
        Retun_less_4per = []
        Retun_less_8per = []
        Retun_less_12per = []

        # 종목별로 빈도 계산
        for date, details in data.items():
            for stock, stock_data in details['stocks'].items():
                if stock_data['quantity_buy'] > 0:
                    quantity_buy_nonzero.append(stock)
                if stock_data['quantity_sold'] > 0:
                    quantity_sold_nonzero.append(stock)
                if stock_data['return_stock_selling'] < 0:
                    Retun_less_0per.append(stock)
                if stock_data['return_stock_selling'] < -0.04:
                    Retun_less_4per.append(stock)
                if stock_data['return_stock_selling'] < -0.08:
                    Retun_less_8per.append(stock)
                if stock_data['return_stock_selling'] < -0.12:
                    Retun_less_12per.append(stock)
                

        # 빈도 계산
        quantity_buy_nonzero_counter = Counter(quantity_buy_nonzero)
        quantity_sold_nonzero_counter = Counter(quantity_sold_nonzero)
        Retun_less_0per_counter = Counter(Retun_less_0per)
        Retun_less_4per_counter = Counter(Retun_less_4per)
        Retun_less_8per_counter = Counter(Retun_less_8per)
        Retun_less_12per_counter = Counter(Retun_less_12per)

        # 데이터프레임 생성
        quantity_buy_nonzero_df = pd.DataFrame(quantity_buy_nonzero_counter.items(), columns=['Stock', 'Buy'])
        quantity_sold_nonzero_df = pd.DataFrame(quantity_sold_nonzero_counter.items(), columns=['Stock', 'Sell'])
        Retun_less_0per_df = pd.DataFrame(Retun_less_0per_counter.items(), columns=['Stock', 'ROI < 0%'])
        Retun_less_4per_df = pd.DataFrame(Retun_less_4per_counter.items(), columns=['Stock', 'ROI < -4%'])
        Retun_less_8per_df = pd.DataFrame(Retun_less_8per_counter.items(), columns=['Stock', 'ROI < -8%'])
        Retun_less_12per_df = pd.DataFrame(Retun_less_12per_counter.items(), columns=['Stock', 'ROI < -12%'])

        # 데이터프레임 리스트 생성
        dfs = [quantity_buy_nonzero_df, quantity_sold_nonzero_df, 
            Retun_less_0per_df, Retun_less_4per_df, 
            Retun_less_8per_df, Retun_less_12per_df]

        # reduce 함수를 사용하여 모든 데이터프레임 병합
        merged_df = reduce(lambda left, right: pd.merge(left, right, on='Stock', how='outer'), dfs).fillna(0)

        # 'Total Frequency' 열 추가 및 3열로 마무리
        merged_df['Total'] = merged_df[['Buy', 'Sell']].sum(axis=1)

        # 'Total Frequency' 열을 3번째 열로 배치
        cols = merged_df.columns.tolist()  # 기존 열 목록 가져오기
        # 'Total Frequency'를 제거하고, 2번 인덱스 위치에 'Total Frequency'를 삽입하여 새 열 순서 생성
        new_cols = cols[:3] + ['Total'] + cols[3:-1]  
        merged_df = merged_df[new_cols]  # 새 열 순서로 데이터프레임 재구성

        # 'Buy Frequency'로 정렬
        sorted_df = merged_df.sort_values(by='ROI < 0%', ascending=False)

        # sorted_df를 json 파일로 저장
        sorted_df.to_json(json_path, orient='records')