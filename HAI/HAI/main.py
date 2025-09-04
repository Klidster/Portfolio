import re
import nltk
import json
import sqlite3
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.metrics.pairwise import cosine_similarity as sklearn_cosine_similarity

# Ensure the 'punkt' tokenizer is downloaded
nltk.download('punkt')

# Database setup
DATABASE = 'bookings.db'

def initialize_database():
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()
    cursor.execute('''CREATE TABLE IF NOT EXISTS bookings (
                        id INTEGER PRIMARY KEY AUTOINCREMENT,
                        time TEXT NOT NULL,
                        people INTEGER NOT NULL
                     )''')
    conn.commit()
    conn.close()

# Add booking to the database
def add_booking_to_db(time, people):
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()
    cursor.execute('INSERT INTO bookings (time, people) VALUES (?, ?)', (time, people))
    conn.commit()
    conn.close()

# Check availability in the database
def is_booking_available_in_db(time):
    conn = sqlite3.connect(DATABASE)
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM bookings WHERE time = ?', (time,))
    result = cursor.fetchone()
    conn.close()
    return result is None

# Load dataset from a JSON file
def load_dataset(file_path):
    with open(file_path, 'r') as file:
        data = [json.loads(line) for line in file]
    return data

# Cosine similarity
def cosine_similarity(doc1, doc2):
    vectorizer = CountVectorizer().fit_transform([doc1, doc2])
    vectors = vectorizer.toarray()
    similarity = sklearn_cosine_similarity(vectors)
    return similarity[0, 1]

# Identify the document type of a question
def get_document_type(user_input, dataset, threshold=0.5):
    highest_similarity = 0
    matched_entry = None

    for entry in dataset:
        similarity = cosine_similarity(user_input, entry['Question'])
        if similarity > highest_similarity:
            highest_similarity = similarity
            matched_entry = entry

    if highest_similarity >= threshold:
        return matched_entry['Document'], matched_entry['Answer']
    return None, None

# Booking logic
def handle_booking(answer):
    print(f"Chatbot: {answer}")
    booking_time = input("You: ")
    print("Chatbot: For how many people?")
    try:
        num_people = int(input("You: "))
        if is_booking_available_in_db(booking_time):
            add_booking_to_db(booking_time, num_people)
            return "Your booking has been confirmed!"
        else:
            return "Sorry, that time slot is not available."
    except ValueError:
        return "Invalid input for the number of people. Please enter a number."
    
# Replace {name} in the response if user_name is available
def personalise_response(response, user_name):
    if user_name and '{name}' in response:
        return response.replace('{name}', user_name)
    return response

# Handle chatbot response
def handle_response(user_input, dataset, user_name, threshold=0.5):
    if user_input.lower() == 'stop':
        return "Goodbye!", None

    # Handle user name initialization
    if user_name is None:
        name_match = re.search(r"my name is (.+)", user_input, re.IGNORECASE)
        if name_match:
            name = name_match.group(1).strip().capitalize()
            return f"Nice to meet you, {name}!", name
        else:
            return "Hello! I don't know your name yet. Could you please tell me your name by saying 'My name is ...'?", user_name

    # Determine document type
    document_type, answer = get_document_type(user_input, dataset, threshold)
    if document_type == "Booking":
        return handle_booking(answer)

    
    response = answer if answer else "Could you rephrase this please?"
    # Replace {name} in the response
    response = personalise_response(response, user_name)
    return response

# Main chatbot loop
def main(file_path='COMP3074-CW1-Dataset.json'):
    initialize_database()
    dataset = load_dataset(file_path)
    user_name = None
    similarity_threshold = 0.5

    if not dataset:
        print("The dataset is empty or invalid.")
        return

    print("Chatbot: Hello! You can make a booking for our restuarant, ask me anything or type 'STOP' to end the conversation.")

    while True:
        user_input = input("You: ")
        if user_input.lower() == 'stop':
            print("Chatbot: Goodbye!")
            break
        elif user_input.lower().startswith('set threshold'):
            try:
                new_threshold = float(user_input.lower().split('set threshold')[1].strip())
                if 0 <= new_threshold <= 1:
                    similarity_threshold = new_threshold
                    print(f"Chatbot: Similarity threshold set to {similarity_threshold}.")
                else:
                    print("Chatbot: Please enter a value between 0 and 1.")
            except ValueError:
                print("Chatbot: Invalid threshold value. Please enter a number between 0 and 1.")
            continue

        response = handle_response(user_input, dataset, user_name, similarity_threshold)

        if isinstance(response, tuple):
            response_text, user_name = response
            print(f"Chatbot: {response_text}")
        else:
            print(f"Chatbot: {response}")

if __name__ == "__main__":
    main()
