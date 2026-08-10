-- SQLite
select * from Alerts;

-- Delete table data Truncate command not available in SQLite
DELETE FROM Alerts;
DELETE FROM sqlite_sequence WHERE name='Alerts';