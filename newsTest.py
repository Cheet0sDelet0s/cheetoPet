import random
import datetime

today = datetime.date.today()
random.seed(today.toordinal())  # ensures same output on same date

templates = [
    "On {date}, {city} reported {event}.",
    "{event} shocked {city} on {date}.",
    "Residents of {city} were met with {event} this {date}."
]

cities = ["New York", "London", "Tokyo", "Sydney", "Bristol"]
events = [
    "a mysterious blackout",
    "an unexpected festival",
    "record-breaking heat",
    "a surprise parade",
    "a flock of birds blocking traffic"
]

template = random.choice(templates)
city = random.choice(cities)
event = random.choice(events)

story = template.format(date=today.strftime("%B %d, %Y"), city=city, event=event)
print(story)
