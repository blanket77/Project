-- MySQL dump 10.13  Distrib 8.0.36, for Win64 (x86_64)
--
-- Host: localhost    Database: stock_db
-- ------------------------------------------------------
-- Server version	8.0.36

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `lab_assignment2_open`
--

DROP TABLE IF EXISTS `lab_assignment2_open`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `lab_assignment2_open` (
  `Date` datetime DEFAULT NULL,
  `SJM` double DEFAULT NULL,
  `BIO` double DEFAULT NULL,
  `HUM` double DEFAULT NULL,
  `GILD` double DEFAULT NULL,
  `PFE` double DEFAULT NULL,
  `WBD` double DEFAULT NULL,
  `MOS` double DEFAULT NULL,
  `WBA` double DEFAULT NULL,
  `MKTX` double DEFAULT NULL,
  `SBAC` double DEFAULT NULL,
  `BA` double DEFAULT NULL,
  `PAYC` double DEFAULT NULL,
  `LKQ` double DEFAULT NULL,
  `MSCI` double DEFAULT NULL,
  `TSLA` double DEFAULT NULL,
  `NKE` double DEFAULT NULL,
  `VRSN` double DEFAULT NULL,
  `DECK` double DEFAULT NULL,
  `CHRW` double DEFAULT NULL,
  `HII` double DEFAULT NULL,
  `LW` double DEFAULT NULL,
  `APTV` double DEFAULT NULL,
  `CDNS` double DEFAULT NULL,
  `EOG` double DEFAULT NULL,
  `RL` double DEFAULT NULL,
  `FRT` double DEFAULT NULL,
  `TTWO` double DEFAULT NULL,
  `LLY` double DEFAULT NULL,
  KEY `ix_lab_assignment2_open_Date` (`Date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `lab_assignment2_open`
--

LOCK TABLES `lab_assignment2_open` WRITE;
/*!40000 ALTER TABLE `lab_assignment2_open` DISABLE KEYS */;
INSERT INTO `lab_assignment2_open` VALUES ('2024-05-13 00:00:00',115.58999633789062,281.2900085449219,335.4100036621094,66.1500015258789,28.190000534057617,8.300000190734863,29.739999771118164,17.25,206.99000549316406,199.7899932861328,179.0399932861328,174.2100067138672,44.529998779296875,488.239990234375,170,91.2699966430664,171.50999450683594,864.4000244140625,82.95999908447266,253.25999450683594,85.51000213623047,83.20999908447266,289.20001220703125,130.47000122070312,168.27000427246094,102.56999969482422,146.5500030517578,763.5),('2024-05-14 00:00:00',116.4800033569336,288.1700134277344,341.2099914550781,68,28.549999237060547,8.489999771118164,29.889999389648438,18.540000915527344,206.85000610351562,200.1999969482422,180.22999572753906,175,44.900001525878906,490,174.5,92.98999786376953,172.25999450683594,861.2999877929688,84.9800033569336,254.25999450683594,85.12999725341797,84.02999877929688,284.6000061035156,129.42999267578125,166.61000061035156,103,144.00999450683594,751.52001953125),('2024-05-15 00:00:00',115.37999725341797,298.0400085449219,345.25,67.93000030517578,28.489999771118164,8.6899995803833,30.25,18.40999984741211,208.41000366210938,204.4199981689453,180.13999938964844,176.4600067138672,44.130001068115234,490,179.89999389648438,92.29000091552734,169.57000732421875,888,84.2300033569336,251.91000366210938,86.18000030517578,83.68000030517578,287.19000244140625,129,167.25,103.86000061035156,145.77000427246094,764.1500244140625),('2024-05-16 00:00:00',113.63999938964844,299.7900085449219,348.3699951171875,67.01000213623047,28.8700008392334,8.239999771118164,30.200000762939453,17.979999542236328,216.4600067138672,202.05999755859375,177.60000610351562,177.82000732421875,44.189998626708984,488.5299987792969,174.10000610351562,91.69000244140625,169.08999633789062,905.2000122070312,83.19999694824219,251.8699951171875,86.01000213623047,81.79000091552734,293.1000061035156,128.77999877929688,167.6199951171875,103,148.97000122070312,784.7100219726562),('2024-05-17 00:00:00',116,296,355.6000061035156,67.81999969482422,28.899999618530273,8.220000267028809,30.899999618530273,18.34000015258789,217.7100067138672,202.75,183.25,181.1199951171875,44.20000076293945,500.8800048828125,173.5500030517578,92.01000213623047,170.9600067138672,889.9199829101562,83.22000122070312,256.989990234375,87.05999755859375,82.44999694824219,290,128.77999877929688,166.6300048828125,101.79000091552734,151.63999938964844,772.8900146484375),('2024-05-20 00:00:00',115.12999725341797,293,356.20001220703125,67.72000122070312,28.65999984741211,8.050000190734863,30.56999969482422,17.969999313354492,214.7899932861328,199.6300048828125,184.64999389648438,182.3000030517578,44.79999923706055,502.5400085449219,177.55999755859375,92.25,170.82000732421875,888.6300048828125,84.11000061035156,256.989990234375,86.8499984741211,81.8499984741211,288.739990234375,130.22999572753906,167.52999877929688,102.13999938964844,147.10000610351562,766.9000244140625),('2024-05-21 00:00:00',114.20999908447266,291.2099914550781,356.9599914550781,67.80000305175781,28.469999313354492,8.09000015258789,30.65999984741211,17.75,214.22999572753906,198.41000366210938,185.3000030517578,179.55999755859375,44.650001525878906,511.2900085449219,175.50999450683594,91.56999969482422,172.0500030517578,899.6099853515625,84.29000091552734,256.3900146484375,86.79000091552734,81.41999816894531,292.2300109863281,129.44000244140625,166.80999755859375,100.68000030517578,150,792.4600219726562),('2024-05-22 00:00:00',111.54000091552734,288.9200134277344,355.17999267578125,67.18000030517578,28.469999313354492,7.869999885559082,30.549999237060547,16.549999237060547,217.5,196.8800048828125,184.60000610351562,179.61000061035156,44.560001373291016,507.6300048828125,182.85000610351562,91.94999694824219,174.7899932861328,902.3400268554688,82.02999877929688,253.0800018310547,86.80000305175781,81.55000305175781,291.9200134277344,128.4499969482422,167.32000732421875,101.13999938964844,151.3800048828125,801),('2024-05-23 00:00:00',111.5999984741211,292.57000732421875,354.0899963378906,67.55999755859375,29.43000030517578,8.0600004196167,32,16.399999618530273,218.25999450683594,193.0399932861328,185.7899932861328,180.9499969482422,43.77000045776367,505.55999755859375,181.8000030517578,92.44999694824219,174.41000366210938,901.239990234375,84.01000213623047,253.91000366210938,88.77999877929688,82.08000183105469,297.6700134277344,126.66000366210938,160.02000427246094,100.94000244140625,153.52000427246094,810),('2024-05-24 00:00:00',110.55999755859375,292.04998779296875,353.30999755859375,66.37999725341797,28.670000076293945,7.739999771118164,31.149999618530273,16.049999237060547,214.3300018310547,188.58999633789062,173.1999969482422,173.8800048828125,42.689998626708984,492.8500061035156,174.83999633789062,91.7699966430664,171.39999389648438,1014.9199829101562,84.79000091552734,254.6300048828125,87.98999786376953,80.72000122070312,293.70001220703125,124.62999725341797,168.57000732421875,99.47000122070312,152.44000244140625,808),('2024-05-28 00:00:00',109.5,287,349.260009765625,65.30999755859375,28.799999237060547,7.730000019073486,31.079999923706055,16.030000686645508,215.60000610351562,188.92999267578125,174.72999572753906,171,42.869998931884766,493,176.39999389648438,91.73999786376953,170,1040,86.58999633789062,257.8500061035156,88.8499984741211,82.62999725341797,293.07000732421875,123.9000015258789,174.57000732421875,99,154.97000122070312,809),('2024-05-29 00:00:00',107.98999786376953,283.19000244140625,345.9599914550781,63.5,28.15999984741211,7.75,30.579999923706055,15.1899995803833,203.9499969482422,186.02000427246094,173.61000061035156,164.16000366210938,42.47999954223633,491.0799865722656,174.19000244140625,91.68000030517578,170.00999450683594,1077,85.2300033569336,252.5399932861328,87.81999969482422,81.51000213623047,291.7900085449219,125.97000122070312,180.41000366210938,97,155.72000122070312,803);
/*!40000 ALTER TABLE `lab_assignment2_open` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2024-07-16 13:18:17