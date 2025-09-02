using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ObjectInteractor : MonoBehaviour
{
    [Header("Pick-Up Settings")]
    public LayerMask pickUpMask;
    public Transform holdPosition;

    [Header("Collision Blocking")]
    public LayerMask blockingMask;

    private GameObject heldObject;
    private Rigidbody heldRb;
    private BoxCollider heldBox;

    void Update()
    {

        if (Input.GetButtonDown("Fire1") && heldObject == null)
        {
            if (Physics.Raycast(transform.position, transform.forward,
                                out var hit, 10f, pickUpMask))
            {
                heldObject = hit.collider.gameObject;
                heldRb = heldObject.GetComponent<Rigidbody>();
                heldBox = heldObject.GetComponent<BoxCollider>();
                if (heldRb == null || heldBox == null)
                {
                    heldObject = null;
                    return;
                }

                heldRb.useGravity = false;
                heldRb.isKinematic = false;
                heldRb.interpolation = RigidbodyInterpolation.Interpolate;
                heldRb.collisionDetectionMode = CollisionDetectionMode.ContinuousDynamic;
            }
        }

        else if (Input.GetButtonUp("Fire1") && heldObject != null)
        {
            Release();
        }

        else if (Input.GetButtonUp("Fire2") && heldObject != null)
        {
            heldRb.useGravity = true;
            heldRb.AddForce(transform.forward * 10f, ForceMode.Impulse);
            Release();
        }
    }

    void FixedUpdate()
    {
        if (heldRb == null) return;


        Vector3 start = heldRb.position;
        Vector3 target = holdPosition.position;
        Vector3 delta = target - start;
        float dist = delta.magnitude;
        if (dist < 0.001f) return;

        Vector3 halfExtents = Vector3.Scale(heldBox.size * 0.5f,
                                            heldObject.transform.lossyScale);

        if (!Physics.BoxCast(start, halfExtents, delta.normalized,
                             out var hit, heldRb.rotation,
                             dist, blockingMask, QueryTriggerInteraction.Ignore))
        {

            heldRb.MovePosition(target);
            heldRb.MoveRotation(holdPosition.rotation);
        }
        else
        {
            float safeDist = Mathf.Max(hit.distance - 0.01f, 0f);
            Vector3 safePos = start + delta.normalized * safeDist;
            heldRb.MovePosition(safePos);
        }
    }

    void Release()
    {
        heldRb.useGravity = true;
        heldObject = null;
        heldRb = null;
        heldBox = null;
    }
}



