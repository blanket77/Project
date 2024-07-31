import sqlite3 
import pandas as pd
import os
from dotenv import load_dotenv
import pymysql
from sshtunnel import SSHTunnelForwarder

load_dotenv()

# SSH connection details
ssh_host = os.getenv('SSH_HOST')
ssh_port = int(os.getenv('SSH_PORT'))
ssh_user = os.getenv('SSH_USER')
ssh_pem_file = os.getenv('SSH_PEM_FILE')

# Database connection details
db_host = os.getenv('DB_HOST')
db_port = int(os.getenv('DB_PORT'))
db_user = os.getenv('DB_USER')
db_password = os.getenv('DB_PASSWORD')
db_name = os.getenv('DB_NAME')

# Path to your CSV file
csv_file = os.getenv('CSV_FILE')

class utilities:
    def fetch_data_from_db(database, table_name):
        conn = sqlite3.connect(database)
        query = f"SELECT * FROM {table_name}"
        df = pd.read_sql(query, conn)
        conn.close()
        return df

    def fetch_table_names(database):
        conn = sqlite3.connect(database)
        cursor = conn.cursor()
        cursor.execute("SELECT name FROM sqlite_master WHERE type='table'")
        tables = cursor.fetchall()
        conn.close()
        return [table[0] for table in tables]

    def fetch_ssh_table_names(database):
        with SSHTunnelForwarder(
                (ssh_host, ssh_port),
                ssh_username=ssh_user,
                ssh_pkey=ssh_pem_file,
                remote_bind_address=(db_host, db_port)
        ) as tunnel:

            # Connect to the database via the SSH tunnel
            connection = pymysql.connect(
                host='127.0.0.1',  # This is localhost because we use SSH tunneling
                port=tunnel.local_bind_port,
                user=db_user,
                password=db_password,
                database=db_name
            )

            # Execute SELECT * FROM ratio query
            with connection.cursor() as cursor:
                sql = "SELECT name FROM stock_info"
                cursor.execute(sql)
                result = cursor.fetchall()

                # Print or process the fetched data
                for row in result:
                    print(row)  # Or process each row as needed
        if 'connection' in locals() and connection.open:
            connection.close()
        return [row[0] for row in result]

    def fetch_ohlcv_from_ssh_db(self, table_name):
        with SSHTunnelForwarder(
                (ssh_host, ssh_port),
                ssh_username=ssh_user,
                ssh_pkey=ssh_pem_file,
                remote_bind_address=(db_host, db_port)
        ) as tunnel:

            # Connect to the database via the SSH tunnel
            connection = pymysql.connect(
                host='127.0.0.1',  # This is localhost because we use SSH tunneling
                port=tunnel.local_bind_port,
                user=db_user,
                password=db_password,
                database=db_name
            )

            query = f"SELECT o.*, si.name AS stock_name " \
                    f"FROM ohlcv AS o " \
                    f"INNER JOIN stock_info AS si ON si.id = o.stock_id " \
                    f"WHERE si.name = '{table_name}'"
            with connection.cursor() as cursor:
                cursor.execute(query)
                results = cursor.fetchall()
                # Convert results to DataFrame
                df = pd.DataFrame(results, columns=[desc[0] for desc in cursor.description])
        df = df.rename(columns={"market_date": 'datetime'})
        if 'connection' in locals() and connection.open:
            connection.close()
        return df

    def fetch_stock_ratio_from_ssh_db(self, table_name):
        with SSHTunnelForwarder(
                (ssh_host, ssh_port),
                ssh_username=ssh_user,
                ssh_pkey=ssh_pem_file,
                remote_bind_address=(db_host, db_port)
        ) as tunnel:

            # Connect to the database via the SSH tunnel
            connection = pymysql.connect(
                host='127.0.0.1',  # This is localhost because we use SSH tunneling
                port=tunnel.local_bind_port,
                user=db_user,
                password=db_password,
                database=db_name
            )

            query = f"SELECT o.*, si.name AS stock_name " \
                    f"FROM ratio AS o " \
                    f"INNER JOIN stock_info AS si ON si.id = o.stock_id " \
                    f"WHERE si.name = '{table_name}'"
            with connection.cursor() as cursor:
                cursor.execute(query)
                results = cursor.fetchall()
                # Convert results to DataFrame
                df = pd.DataFrame(results, columns=[desc[0] for desc in cursor.description])
        df = df.rename(columns={'profitability': 'Profitability', 'recorded_date': 'Date',
                                'safety': "Safety", 'growth': 'Growth', 'quality': 'Quality',})
        if 'connection' in locals() and connection.open:
            connection.close()
        return df

    def fetch_data_from_ssh_db(self, database, table_name):
        try:
            if 'ohlcv' in database:
                return self.fetch_ohlcv_from_ssh_db(table_name)
            if 'ratio' in database:
                return self.fetch_stock_ratio_from_ssh_db(table_name)
        except:
            print(f'no {table_name} in {database}')
            return None


if __name__ == '__main__':
    t = utilities()
    k = t.fetch_stock_ratio_from_ssh_db('A')
    j = t.fetch_data_from_ssh_db('ratio', 'AGL')