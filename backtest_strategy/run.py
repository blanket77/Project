from backtest.Static_Asset_Allocation.strategy import Static_Asset_Allocation
from backtest.Tactical_asset_allocation.strategy import Tactical_asset_allocation
from backtest.trend_following.strategy import trend_following
from backtest.longshort_strategy.sma.sma_strategy import sma_longshort
from backtest.longshort_strategy.macd.macd_strategy import macd_longshort
from backtest.Mean_reversion_trading.rsi.rsi_strategy import rsi_mean_reversion
from backtest.Mean_reversion_trading.bb.bb_strategy import bb_mean_reversion
from backtest.Mean_reversion_trading.stochastic_oscillator_strategy import stochastic_oscillator

# gogo = Static_Asset_Allocation(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], 1000000, 'monthly', '2020-07-01', '2022-12-30')
# gogo.display()

# gogo1 = Tactical_asset_allocation(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], 1000000, 'monthly', '2020-07-01', '2022-12-30', 3,1) 
# gogo1.display()

# gogo2 = trend_following(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], 1000000, '2020-07-01', '2022-12-30',50)
# gogo2.display()

# gogo3 = sma_longshort(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], 1000000, '2020-07-01', '2022-12-30', 100,50)
# gogo3.display()

# gogo4 = macd_longshort(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], 1000000, '2020-07-01', '2022-12-30', 12,26,9)
# gogo4.display()

# gogo5 = rsi_mean_reversion(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], 1000000, '2020-07-01', '2022-12-30', 70,30)
# gogo5.display()

# gogo6 = bb_mean_reversion(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], 1000000, '2020-07-01', '2022-12-30', 20,2,2)
# gogo6.display()

gogo7 = stochastic_oscillator(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], 1000000, '2020-07-01', '2022-12-30', 14,3,3)
gogo7.display()

