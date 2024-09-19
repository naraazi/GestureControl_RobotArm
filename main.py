import serial
import cv2
import mediapipe as mp

# -- general configurations
write_video = False
debug = False
cam_source = "/dev/video2"

if not debug:
    ser = serial.Serial('/dev/ttyUSB0', 9600)

# -- use angle between wrist and index finger to control x-axis
x_min = 90
x_mid = 130
x_max = 180
palm_angle_min = -50
palm_angle_mid = 20

# -- use wrist y to control y axis
y_min = 50
y_mid = 90
y_max = 145
wrist_y_min = 0.3
wrist_y_max = 0.9

# -- use palm size to control z axis
z_min = 103
z_mid = 129
z_max = 180
palm_size_min = 0.1  # -- major (live) problem here
palm_size_max = 0.3  # -- major (live) problem here

claw_open_angle = 110
claw_close_angle = 146

servo_angle = [x_mid, y_mid, z_mid, claw_open_angle]
prev_servo_angle = servo_angle
fist_threshold = 7

mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_hands = mp.solutions.hands

cap = cv2.VideoCapture(cam_source)

# -- video writer
if write_video:
    fourcc = cv2.VideoWriter_fourcc(*'XVID')
    out = cv2.VideoWriter('output.avi', fourcc, 60.0, (640, 480))

clamp = lambda n, minn, maxn: max(min(maxn, n), minn)
map_range = lambda x, in_min, in_max, out_min, out_max: abs(
    (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min)


# -- check if the hand is a fist
def is_fist(hand_landmarks, palm_size):
    # -- calculate the distance between the wrist and the each finger tip
    distance_sum = 0
    WRIST = hand_landmarks.landmark[0]
    for i in [7, 8, 11, 12, 15, 16, 19, 20]:
        distance_sum += ((WRIST.x - hand_landmarks.landmark[i].x) ** 2 + \
                         (WRIST.y - hand_landmarks.landmark[i].y) ** 2 + \
                         (WRIST.z - hand_landmarks.landmark[i].z) ** 2) ** 0.5
    return distance_sum / palm_size < fist_threshold


def landmark_to_servo_angle(hand_landmarks):
    servo_angle = [x_mid, y_mid, z_mid, claw_open_angle]
    WRIST = hand_landmarks.landmark[0]
    INDEX_FINGER_MCP = hand_landmarks.landmark[5]
    # -- calculate the distance between the wrist and the index finger
    palm_size = ((WRIST.x - INDEX_FINGER_MCP.x) ** 2 + (WRIST.y - INDEX_FINGER_MCP.y) ** 2 + (
                WRIST.z - INDEX_FINGER_MCP.z) ** 2) ** 0.5

    if is_fist(hand_landmarks, palm_size):
        servo_angle[3] = claw_close_angle
    else:
        servo_angle[3] = claw_open_angle

    # -- calculate x angle
    distance = palm_size
    angle = (WRIST.x - INDEX_FINGER_MCP.x) / distance  # -- calculate the radian between the wrist and the index finger
    angle = int(angle * 180 / 3.1415926)  # -- convert radian to degree
    angle = clamp(angle, palm_angle_min, palm_angle_mid)
    servo_angle[0] = map_range(angle, palm_angle_min, palm_angle_mid, x_max, x_min)

    # -- calculate y angle
    wrist_y = clamp(WRIST.y, wrist_y_min, wrist_y_max)
    servo_angle[1] = map_range(wrist_y, wrist_y_min, wrist_y_max, y_max, y_min)

    # -- calculate z angle
    palm_size = clamp(palm_size, palm_size_min, palm_size_max)
    servo_angle[2] = map_range(palm_size, palm_size_min, palm_size_max, z_max, z_min)

    # -- float to int
    servo_angle = [int(i) for i in servo_angle]

    return servo_angle


with mp_hands.Hands(model_complexity=0, min_detection_confidence=0.5, min_tracking_confidence=0.5) as hands:
    while cap.isOpened():
        success, image = cap.read()
        if not success:
            print("Ignorando cena vazia.")
            continue

        image.flags.writeable = False
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = hands.process(image)

        # -- draw the hand annotations on the image.
        image.flags.writeable = True
        image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
        if results.multi_hand_landmarks:
            if len(results.multi_hand_landmarks) == 1:
                hand_landmarks = results.multi_hand_landmarks[0]
                servo_angle = landmark_to_servo_angle(hand_landmarks)

                if servo_angle != prev_servo_angle:
                    print("Ângulo dos servos: ", servo_angle)
                    prev_servo_angle = servo_angle
                    if not debug:
                        ser.write(bytearray(servo_angle))
            else:
                print("Mais de uma mão detectada.")
            for hand_landmarks in results.multi_hand_landmarks:
                mp_drawing.draw_landmarks(
                    image,
                    hand_landmarks,
                    mp_hands.HAND_CONNECTIONS,
                    mp_drawing_styles.get_default_hand_landmarks_style(),
                    mp_drawing_styles.get_default_hand_connections_style())

        # -- flip the image horizontally for a selfie-view display.
        image = cv2.flip(image, 1)
        cv2.putText(image, str(servo_angle), (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2,
                    cv2.LINE_AA)  # -- show servo angle
        cv2.imshow('Retorno', image)

        if write_video:
            out.write(image)
        if cv2.waitKey(5) & 0xFF == 27:  # -- ESC to exit
            if write_video:
                out.release()
            break
cap.release()
