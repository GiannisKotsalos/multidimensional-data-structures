import pandas as pd

# Read the CSV file
df = pd.read_csv('data_movies_clean.csv')

# Print first few lines
print("First 5 rows:")
print(df.head())
print("\n" + "="*50 + "\n")
print("Shape of dataframe:", df.shape)